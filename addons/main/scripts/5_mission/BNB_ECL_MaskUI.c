// Organisation: Bullets'n'Bandages
// Author:       Bushy <contact@bushy.dev>
// Version:      v1.1.3
// Modified:     2026-07-18
//
// BNB_ECL_MaskUI.c - keypad UI companion to mask_code_display. With masking
// on, typed digits accumulate in a client-local buffer and NO display ever
// shows them - the typer's own overlay renders one * per digit and every
// synced/broadcast value carries same-length zeros. The real PIN leaves the
// client only in the confirm submit. OFF (default) leaves upstream untouched.

#ifdef BM_CodeLock
modded class BM_CodeLockUI
{
    protected string m_BNB_LocalCode = "";

    override void OnShow()
    {
        m_BNB_LocalCode = "";
        super.OnShow();
    }

    // The admin "Change" verb clears the entry server-side; the local buffer
    // must reset with it or stale digits prefix the newly set passcode.
    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (BNB_EclBridgeSettings.MaskCodeDisplay() && w && w == m_AdminChange)
            m_BNB_LocalCode = "";
        return super.OnClick(w, x, y, button);
    }

    // Masked keystrokes update the local buffer and send only zeros on the
    // display-sync wire (still re-arms the lock's server-side sleep timer).
    override protected void EnterDigit(int digit)
    {
        if (!BNB_EclBridgeSettings.MaskCodeDisplay())
        {
            super.EnterDigit(digit);
            return;
        }
        BM_CodeLock maskLock = BM_CodeLock.Cast(m_Target);
        if (!maskLock)
            return;
        if (digit == -1)
        {
            maskLock.PlayRandomButtonSound();
            if (m_BNB_LocalCode.Length() > 0)
                m_BNB_LocalCode = m_BNB_LocalCode.Substring(0, m_BNB_LocalCode.Length() - 1);
        }
        else
        {
            if (m_BNB_LocalCode.Length() < 6)
            {
                m_BNB_LocalCode += digit.ToString();
                maskLock.PlayRandomButtonSound();
            }
        }
        string wire = "";
        for (int i = 0; i < m_BNB_LocalCode.Length(); i++)
        {
            wire += "0";
        }
        GetGame().RPCSingleParam(maskLock, BM_CODELOCK_RPC_TYPED_REQUEST, new Param1<string>(wire), true, null);
    }

    // Masked confirm submits the local buffer through the upstream RPCs -
    // the only wire that ever carries the real code, client-to-server.
    override protected void TryCode()
    {
        if (!BNB_EclBridgeSettings.MaskCodeDisplay())
        {
            super.TryCode();
            return;
        }
        BM_CodeLock tryLock = BM_CodeLock.Cast(m_Target);
        if (!tryLock)
            return;
        if (m_BNB_LocalCode.Length() != 6)
            return;
        tryLock.PlayEnterSound();
        if (m_Player && m_Player.IsBMCodeLockAdminChangingPass())
        {
            GetGame().RPCSingleParam(tryLock, BM_CODELOCK_RPC_ADMIN_SET_CODE_REQUEST, new Param1<string>(m_BNB_LocalCode), true, null);
            m_Player.SetBMCodeLockAdminChangingPass(false);
            m_BNB_LocalCode = "";
            return;
        }
        if (m_CreatingPass)
        {
            GetGame().RPCSingleParam(tryLock, BM_CODELOCK_RPC_CREATE_REQUEST, new Param1<string>(m_BNB_LocalCode), true, null);
            CloseCodeLockUI();
            return;
        }
        GetGame().RPCSingleParam(tryLock, BM_CODELOCK_RPC_TRY_REQUEST, new Param1<string>(m_BNB_LocalCode), true, null);
    }

    // Masked entry shows * per digit even to the typer; SETTING a PIN (create
    // or admin change) shows the typer real digits. Bystanders see blocks.
    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (BNB_EclBridgeSettings.MaskCodeDisplay() && m_Screen)
        {
            bool bnbSettingPin = m_CreatingPass || (m_Player && m_Player.IsBMCodeLockAdminChangingPass());
            if (bnbSettingPin)
            {
                m_Screen.SetText(m_BNB_LocalCode);
                return;
            }
            string stars = "";
            for (int si = 0; si < m_BNB_LocalCode.Length(); si++)
            {
                stars += "*";
            }
            m_Screen.SetText(stars);
        }
    }
}
#endif
