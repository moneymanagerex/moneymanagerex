/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Nikolay & Stefano Giorgio

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#ifndef MM_EX_BDDIALOG_H_
#define MM_EX_BDDIALOG_H_

#include "defs.h"
#include <wx/spinbutt.h>
#include <wx/dialog.h>
#include "mmtextctrl.h"
#include "splittransactionsdialog.h"
#include "model/Model_Budgetsplittransaction.h"
#include "model/Model_Billsdeposits.h"

class wxDatePickerCtrl;

class mmBDDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmBDDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmBDDialog();
    mmBDDialog(wxWindow* parent, int bdD, bool edit, bool enterOccur);
    int GetTransID()
    {
        return transID_;
    }

    void SetDialogHeader(const wxString& header);

private:
    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

    void CreateControls();

    // utility functions
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnCategs(wxCommandEvent& event);
    void OnPayee(wxCommandEvent& event);
    void OnTo(wxCommandEvent& event);
    void OnTypeChanged(wxCommandEvent& event);
	void OnAttachments(wxCommandEvent& event);

    void OnDateChanged(wxDateEvent& event);
    void dataToControls();
    void updateControlsForTransType();
    void addPayee(wxString payee, int categID, int subcategID );
    void OnAccountName(wxCommandEvent& event);
    void OnSplitChecked(wxCommandEvent& event);
    void OnAutoExecutionUserAckChecked(wxCommandEvent& event);
    void OnAutoExecutionSilentChecked(wxCommandEvent& event);
    void OnCalendarSelChanged(wxCalendarEvent& event);
    void OnTextEntered(wxCommandEvent& event);
    int transID_;

    wxTextCtrl *textNumber_;
    mmTextCtrl *textAmount_;
    mmTextCtrl *toTextAmount_;
    wxTextCtrl *textNotes_;
    wxTextCtrl* textCategory_;
    wxTextCtrl* textNumRepeats_;
    wxButton* bCategory_;
    wxButton* bPayee_;
    wxButton* bAccount_;
	wxButton* bAttachments_;
    wxCheckBox* cSplit_;
    wxCheckBox* cAdvanced_;
    bool payeeUnknown_;

    wxChoice* choiceStatus_;
    wxChoice* transaction_type_;

    bool edit_;
    bool m_new_bill;
    bool m_transfer;
    bool enterOccur_;

    wxDatePickerCtrl* dpc_;         // dpcTransDate_
    wxDatePickerCtrl* dpcNextOccDate_;       // dpcNextOccDate_
    wxCalendarCtrl* calendarCtrl_;
    wxChoice* itemRepeats_;
    wxCheckBox* itemCheckBoxAutoExeUserAck_;
    wxCheckBox* itemCheckBoxAutoExeSilent_;
    bool autoExecuteUserAck_;
    bool autoExecuteSilent_;

    Model_Billsdeposits::Bill_Data m_bill_data;
    std::vector<Split> local_splits;

    bool m_advanced;
    bool categUpdated_;
	bool skip_attachments_init_;
    int prevType_;
    std::vector<wxString> frequentNotes_;

    wxString payeeWithdrawalTip_;
    wxString amountNormalTip_;
    wxString amountTransferTip_;
    wxSpinButton *spinNextOccDate_;
    wxSpinButton *spinTransDate_;

    void displayControlsForType(Model_Billsdeposits::TYPE transType, bool enableAdvanced = false);
    void resetPayeeString();
    void setTooltips();
    void setCategoryLabel();
    void OnAdvanceChecked(wxCommandEvent& event);
    void SetTransferControls(bool transfers = false);
    void SetAdvancedTransferControls(bool advanced = false);
    void SetSplitControls(bool split = false);
    void OnTransDateForward(wxSpinEvent& event);
    void OnTransDateBack(wxSpinEvent& event);
    void OnNextOccurDateForward(wxSpinEvent& event);
    void OnNextOccurDateBack(wxSpinEvent& event);
    void SetNewDate(wxDatePickerCtrl* dpc, bool forward = true);
    void OnFrequentUsedNotes(wxCommandEvent& event);
    void onNoteSelected(wxCommandEvent& event);

    wxStaticText* staticTimesRepeat_;
    wxStaticText* staticTextRepeats_;
    wxButton* bSetNextOccurDate_;
    void OnRepeatTypeChanged(wxCommandEvent& event);
    void OnsetNextRepeatDate(wxCommandEvent& event);
    void OnPeriodChange(wxCommandEvent& event);
    void setRepeatDetails();

    void activateSplitTransactionsDlg();

    enum
    {
        ID_DIALOG_TRANS_TYPE = wxID_HIGHEST + 200,
        ID_DIALOG_TRANS_BUTTONCATEGS,
        ID_DIALOG_TRANS_STATIC_ACCOUNT,
        ID_DIALOG_TRANS_TEXTNUMBER,
        ID_DIALOG_TRANS_BUTTONDATE,
        ID_DIALOG_TRANS_TEXTNOTES,
        ID_DIALOG_TRANS_TEXTAMOUNT,
        ID_DIALOG_TRANS_TOTEXTAMOUNT,
        ID_DIALOG_TRANS_STATIC_PAYEE,
        ID_DIALOG_TRANS_BUTTONPAYEE,
        ID_DIALOG_TRANS_BUTTONTO,
        ID_DIALOG_TRANS_STATUS,
        ID_DIALOG_TRANS_ADVANCED_CHECKBOX,
        ID_DIALOG_TRANS_ADVANCED_FROM,
        ID_DIALOG_TRANS_ADVANCED_TO,
        ID_DIALOG_TRANS_SPLITCHECKBOX,
        ID_DIALOG_TRANS_BUTTONTRANSNUM,
        ID_DIALOG_TRANS_PAYEECOMBO,
        ID_DIALOG_TRANS_BUTTON_FREQENTNOTES,
        ID_DIALOG_TRANS_DATE_SPINNER,
        ID_DIALOG_BD,
        ID_DIALOG_BD_COMBOBOX_ACCOUNTNAME,
        ID_DIALOG_BD_BUTTON_NEXTOCCUR,
        ID_DIALOG_BD_COMBOBOX_REPEATS,
        ID_DIALOG_BD_TEXTCTRL_NUM_TIMES,
        ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK,
        ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT,
        ID_DIALOG_BD_CALENDAR,
        ID_DIALOG_BD_REPEAT_DATE_SPINNER,
        ID_PANEL_REPORTS_HTMLWINDOW,
        ID_PANEL_REPORTS_HEADER_PANEL,
        ID_PANEL_REPORTS_STATIC_HEADER,
    };
};

#endif
