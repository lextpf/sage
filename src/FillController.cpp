#ifdef USE_QT_UI

#include "FillController.h"
#include "Clipboard.h"

#include <QtCore/QThread>
#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>

namespace sage {

// Only one controller can own the global hooks at a time.
FillController* FillController::s_instance = nullptr;

FillController::FillController(QObject* parent)
    : QObject(parent)
{
    m_TimeoutTimer.setInterval(1000);
    connect(&m_TimeoutTimer, &QTimer::timeout, this, &FillController::onTimeoutTick);
}

FillController::~FillController()
{
    // Ensure hooks are removed even if the caller forgot to cancel().
    if (m_State != State::Idle)
        cancel();
}

bool FillController::isArmed() const
{
    return m_State == State::ArmedUsername || m_State == State::ArmedPassword;
}

QString FillController::fillStatusText() const
{
    return m_StatusText;
}

int FillController::countdownSeconds() const
{
    return m_RemainingSeconds;
}

FillController::State FillController::state() const
{
    return m_State;
}

void FillController::transitionTo(State newState)
{
    bool wasArmed = isArmed();
    m_State = newState;
    bool nowArmed = isArmed();

    updateStatusText();

    // Only emit armedChanged when the armed/disarmed boundary is crossed,
    // not on every internal state change (e.g. ArmedUsername -> ArmedPassword).
    if (wasArmed != nowArmed)
        emit armedChanged();
}

void FillController::updateStatusText()
{
    QString prev = m_StatusText;
    switch (m_State)
    {
        case State::Idle:
            m_StatusText.clear();
            break;
        case State::ArmedUsername:
            m_StatusText = QStringLiteral("Ctrl+Click to fill username");
            break;
        case State::ArmedPassword:
            m_StatusText = QStringLiteral("Ctrl+Click to fill password");
            break;
        case State::Typing:
            m_StatusText = QStringLiteral("Typing...");
            break;
    }
    if (m_StatusText != prev)
        emit fillStatusTextChanged();
}

void FillController::arm(int recordIndex,
                         const std::vector<sage::VaultRecord>& records,
                         const sage::basic_secure_string<wchar_t, sage::locked_allocator<wchar_t>>& masterPw)
{
    // If already armed (e.g. user clicked a different record), tear down
    // the previous session before starting a new one.
    if (m_State != State::Idle)
        cancel();

    // Borrow pointers - the caller (Backend) owns these and must
    // keep them alive until fillCompleted / fillCancelled / fillError.
    m_RecordIndex    = recordIndex;
    m_Records        = &records;
    m_MasterPw       = &masterPw;
    m_RemainingSeconds = FILL_TIMEOUT_SECONDS;
    m_PendingTarget  = TypeTarget::Username;

    // Register as the singleton so the static hook callbacks can find us.
    s_instance = this;
    installHooks();
    transitionTo(State::ArmedUsername);

    emit countdownSecondsChanged();
    m_TimeoutTimer.start();
}

void FillController::cancel()
{
    if (m_State == State::Idle)
        return;

    m_TimeoutTimer.stop();
    removeHooks();
    transitionTo(State::Idle);

    // Clear borrowed pointers so we don't dangle.
    m_RecordIndex      = -1;
    m_Records          = nullptr;
    m_MasterPw         = nullptr;
    m_RemainingSeconds = 0;
    emit countdownSecondsChanged();
    emit fillCancelled();
}

void FillController::onTimeoutTick()
{
    // Don't tick while idle or mid-type - the timer should already be
    // stopped in those states, but guard defensively.
    if (m_State == State::Idle || m_State == State::Typing)
        return;

    m_RemainingSeconds--;
    emit countdownSecondsChanged();

    // Auto-cancel if the user hasn't clicked within the timeout window.
    if (m_RemainingSeconds <= 0)
        cancel();
}

void FillController::installHooks()
{
    // WH_MOUSE_LL / WH_KEYBOARD_LL are global low-level hooks that intercept
    // input system-wide. We pass nullptr for hMod and 0 for dwThreadId so
    // the hooks apply to all threads on the desktop.
    m_MouseHook    = SetWindowsHookExW(WH_MOUSE_LL,    mouseHookProc,    nullptr, 0);
    m_KeyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, keyboardHookProc, nullptr, 0);
}

void FillController::removeHooks()
{
    if (m_MouseHook)
    {
        UnhookWindowsHookEx(m_MouseHook);
        m_MouseHook = nullptr;
    }
    if (m_KeyboardHook)
    {
        UnhookWindowsHookEx(m_KeyboardHook);
        m_KeyboardHook = nullptr;
    }
    // Clear the singleton so stale hook callbacks (if any are still in
    // the message queue) won't dereference a dead pointer.
    s_instance = nullptr;
}

LRESULT CALLBACK FillController::mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && s_instance && wParam == WM_LBUTTONDOWN)
    {
        bool ctrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

        if (ctrlDown && (s_instance->m_State == State::ArmedUsername ||
                         s_instance->m_State == State::ArmedPassword))
        {
            // Check modifier keys for field override:
            //   Shift  -> force password
            //   Alt    -> force username
            //   Neither -> follow the current state
            bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
            bool altDown   = (GetAsyncKeyState(VK_MENU)  & 0x8000) != 0;

            TypeTarget target;
            if (shiftDown)
            {
                target = TypeTarget::Password;
            }
            else if (altDown)
            {
                target = TypeTarget::Username;
            }
            else
            {
                target = (s_instance->m_State == State::ArmedUsername)
                    ? TypeTarget::Username : TypeTarget::Password;
            }

            s_instance->m_PendingTarget = target;

            // Queue performType on the Qt event loop - we can't do Qt work
            // or blocking calls inside a low-level hook callback.
            QMetaObject::invokeMethod(s_instance, "performType", Qt::QueuedConnection);

            // Return 1 to swallow the click so the target app doesn't
            // receive it (prevents accidental button presses behind the cursor).
            return 1;
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK FillController::keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && s_instance && wParam == WM_KEYDOWN)
    {
        auto* khs = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        // Esc while armed -> cancel. Queued to avoid blocking the hook.
        if (khs->vkCode == VK_ESCAPE &&
            (s_instance->m_State == State::ArmedUsername ||
             s_instance->m_State == State::ArmedPassword))
        {
            QMetaObject::invokeMethod(s_instance, "cancel", Qt::QueuedConnection);
            return 1;
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void FillController::performType()
{
    if (m_State != State::ArmedUsername && m_State != State::ArmedPassword)
        return;

    TypeTarget target = m_PendingTarget;
    transitionTo(State::Typing);
    m_TimeoutTimer.stop();

    // Poll for Ctrl release before sending keystrokes. If we type while
    // Ctrl is still held, the target app interprets them as Ctrl+shortcuts
    // (e.g. Ctrl+A = select-all instead of typing 'a').
    auto* poll = new QTimer(this);
    poll->setInterval(20);
    int* elapsed = new int(0);
    connect(poll, &QTimer::timeout, this, [this, target, poll, elapsed]()
    {
        *elapsed += 20;
        bool ctrlStillDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        if (ctrlStillDown && *elapsed < 2000)
            return;  // Keep polling - Ctrl not yet released.

        poll->stop();
        poll->deleteLater();
        delete elapsed;

        // Guard against cancel() being called while we were polling.
        if (m_State != State::Typing)
            return;

        // Decrypt the credential on demand - it stays encrypted at rest
        // and is only decrypted into locked memory for the brief moment
        // we need to send the keystrokes.
        sage::DecryptedCredential cred;
        try
        {
            cred = sage::decryptCredentialOnDemand((*m_Records)[m_RecordIndex], *m_MasterPw);
        }
        catch (const std::exception& e)
        {
            emit fillError(QString("Decrypt failed: %1").arg(e.what()));
            cancel();
            return;
        }
        catch (...)
        {
            emit fillError(QStringLiteral("Decrypt failed"));
            cancel();
            return;
        }

        // Type the selected field via synthesized keystrokes (SendInput).
        bool success = false;
        if (target == TypeTarget::Username)
        {
            success = sage::typeSecret(cred.m_Username.data(), (int)cred.m_Username.size(), 0);
        }
        else
        {
            success = sage::typeSecret(cred.m_Password.data(), (int)cred.m_Password.size(), 0);
        }

        // Wipe plaintext immediately after typing.
        cred.cleanse();

        if (!success)
        {
            emit fillError(QStringLiteral("Failed to send keystrokes"));
            cancel();
            return;
        }

        QString service = QString::fromUtf8((*m_Records)[m_RecordIndex].m_Platform.c_str());

        if (target == TypeTarget::Username)
        {
            // Username typed - reset the countdown and transition to
            // ArmedPassword so the user can Ctrl+Click the password field.
            m_RemainingSeconds = FILL_TIMEOUT_SECONDS;
            emit countdownSecondsChanged();
            transitionTo(State::ArmedPassword);
            m_TimeoutTimer.start();
        }
        else
        {
            // Password typed - both phases complete. Tear down hooks and
            // notify the backend so it can restore the window.
            m_TimeoutTimer.stop();
            removeHooks();
            transitionTo(State::Idle);
            m_RecordIndex      = -1;
            m_Records          = nullptr;
            m_MasterPw         = nullptr;
            m_RemainingSeconds = 0;
            emit countdownSecondsChanged();
            emit fillCompleted(QString("Filled credentials for '%1'").arg(service));
        }
    });
    poll->start();
}

} // namespace sage

#endif // USE_QT_UI
