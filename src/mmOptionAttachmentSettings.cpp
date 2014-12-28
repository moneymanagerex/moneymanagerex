#include "mmOptionAttachmentSettings.h"
#include "attachmentdialog.h"
#include "util.h"
#include "constants.h"
#include "paths.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(mmOptionAttachmentSettings, wxPanel)
    EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_ATTACHMENTSFOLDER, mmOptionAttachmentSettings::OnAttachmentsButton)
    EVT_MENU_RANGE(wxID_HIGHEST, wxID_HIGHEST + 9, mmOptionAttachmentSettings::OnAttachmentsMenu)
    EVT_TEXT(ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT, mmOptionAttachmentSettings::OnAttachmentsPathChanged)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_CHECKBOX_ATTACHMENTSSUBFOLDER, mmOptionAttachmentSettings::OnAttachmentsSubfolderChanged)
wxEND_EVENT_TABLE()
/*******************************************************/

mmOptionAttachmentSettings::mmOptionAttachmentSettings()
{

}

mmOptionAttachmentSettings::mmOptionAttachmentSettings(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create(parent, id, pos, size, style, name);
}

mmOptionAttachmentSettings::~mmOptionAttachmentSettings()
{

}

void mmOptionAttachmentSettings::Create(wxWindow *parent
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString &name)
{
    wxBoxSizer* attachmentPanelSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(attachmentPanelSizer);

    //Attachments
    wxStaticBox* attachmentStaticBox = new wxStaticBox(this, wxID_ANY, _("Attachments Settings"));
    SetBoldFont(attachmentStaticBox);
    wxStaticBoxSizer* attachmentStaticBoxSizer = new wxStaticBoxSizer(attachmentStaticBox, wxVERTICAL);

    attachmentPanelSizer->Add(attachmentStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxString OSType = wxPlatformInfo::Get().GetOperatingSystemFamilyName();
    wxString attachmentStaticText_desc = wxString::Format(_("Attachment archive folder for %s only:"), OSType);

    wxStaticText* attachmentStaticText = new wxStaticText(this, wxID_STATIC, attachmentStaticText_desc);
    attachmentStaticBoxSizer->Add(attachmentStaticText, g_flags);
    attachmentStaticText->SetToolTip(_("Every OS type (Win,Mac,Unix) has its attachment folder"));

    wxBoxSizer* attachDefinedSizer = new wxBoxSizer(wxHORIZONTAL);
    attachmentStaticBoxSizer->Add(attachDefinedSizer);

    const wxString attachmentFolder = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:" + mmPlatformType(), "");

    wxTextCtrl* textAttachment = new wxTextCtrl(this
        , ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT
        , ""
        , wxDefaultPosition, wxSize(225, -1), 0);
    wxLogDebug("%s", attachmentFolder);
    textAttachment->ChangeValue(attachmentFolder);

    wxButton* AttachmentsFolderButton = new wxButton(this
        , ID_DIALOG_OPTIONS_BUTTON_ATTACHMENTSFOLDER, "...", wxDefaultPosition, wxSize(25, -1), 0);
    AttachmentsFolderButton->SetToolTip(_("Browse for folder"));

    attachDefinedSizer->Add(textAttachment, g_flags);
    attachDefinedSizer->Add(AttachmentsFolderButton, g_flags);

    wxStaticText* attachmentFolderCurrOSText = new wxStaticText(this
        , ID_DIALOG_OPTIONS_STATICTEXT_ATTACHMENTSTEXT
        , _("Real path:") + "\n" + mmex::getPathAttachment(attachmentFolder));
    attachmentFolderCurrOSText->SetFont(this->GetFont().Smaller());
    attachmentStaticBoxSizer->Add(attachmentFolderCurrOSText, g_flags);

    // Legend
    wxStaticBox* attachmentStaticBoxLegend = new wxStaticBox(this, wxID_ANY, _("Legend "));
    attachmentStaticBoxLegend->SetFont(this->GetFont().Italic());
    wxStaticBoxSizer* attachmentStaticBoxSizerLegend = new wxStaticBoxSizer(attachmentStaticBoxLegend, wxVERTICAL);
    attachmentStaticBoxSizer->Add(attachmentStaticBoxSizerLegend, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxString legend = wxString::Format(_("%s -> User document directory"), ATTACHMENTS_FOLDER_DOCUMENTS);
    legend += "\n" + wxString::Format(_("%s -> User profile folder"), ATTACHMENTS_FOLDER_USERPROFILE);
    legend += "\n" + wxString::Format(_("%s -> Folder of.MMB database file"), ATTACHMENTS_FOLDER_DATABASE);
    legend += "\n" + wxString::Format(_("%s -> MMEX Application data folder"), ATTACHMENTS_FOLDER_APPDATA);
    wxStaticText* legendStaticText = new wxStaticText(this, wxID_STATIC, legend);
    attachmentStaticBoxSizerLegend->Add(legendStaticText);
    //End legend

    //Other OS folders
    wxStaticBox* attachmentStaticBoxInfo = new wxStaticBox(this, wxID_ANY, _("Other OS folders "));
    attachmentStaticBoxInfo->SetFont(this->GetFont().Italic());
    wxStaticBoxSizer* attachmentStaticBoxSizerInfo = new wxStaticBoxSizer(attachmentStaticBoxInfo, wxVERTICAL);
    attachmentStaticBoxSizer->Add(attachmentStaticBoxSizerInfo, wxSizerFlags(g_flagsExpand).Proportion(0));

    const wxString FolderNotSet = _("Not yet set");
    const wxString attachmentFolderWin = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:Win", FolderNotSet);
    const wxString attachmentFolderMac = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:Mac", FolderNotSet);
    const wxString attachmentFolderUnix = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:Uni", FolderNotSet);

    if (mmPlatformType() != "Win")
    {
        wxStaticText* attachmentFolderWinText = new wxStaticText(this
            , wxID_STATIC, _("Windows folder -> ") + attachmentFolderWin.Left(50));
        attachmentFolderWinText->SetToolTip(attachmentFolderWin);
        attachmentStaticBoxSizerInfo->Add(attachmentFolderWinText);
    }

    if (mmPlatformType() != "Mac")
    {
        wxStaticText* attachmentFolderMacText = new wxStaticText(this
            , wxID_STATIC, _("Mac folder -> ") + attachmentFolderMac.Left(50));
        attachmentFolderMacText->SetToolTip(attachmentFolderMac);
        attachmentStaticBoxSizerInfo->Add(attachmentFolderMacText);
    }

    if (mmPlatformType() != "Uni")
    {
        wxStaticText* attachmentFolderUnixText = new wxStaticText(this
            , wxID_STATIC, _("Unix folder -> ") + attachmentFolderUnix.Left(50));
        attachmentFolderUnixText->SetToolTip(attachmentFolderUnix);
        attachmentStaticBoxSizerInfo->Add(attachmentFolderUnixText);
    }
    //End other OS folders

    const wxString LastDBPath = Model_Setting::instance().getLastDbPath();
    const wxFileName fn(LastDBPath);
    const wxString LastDBFileName = fn.FileName(LastDBPath).GetName();
    const wxString subFolder = wxString::Format("MMEX_%s_Attachments", fn.FileName(LastDBPath).GetName());
    const wxString cbAttachmentsSubfolder_desc = _("Create and use Attachments subfolder");

    m_attachments_subfolder = new wxCheckBox(this, ID_DIALOG_OPTIONS_CHECKBOX_ATTACHMENTSSUBFOLDER
        , cbAttachmentsSubfolder_desc, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_attachments_subfolder->SetValue(Model_Infotable::instance().GetBoolInfo("ATTACHMENTSSUBFOLDER", true));
    attachmentStaticBoxSizer->Add(m_attachments_subfolder, g_flags);
    attachmentStaticBoxSizer->Add(new wxStaticText(this
        , wxID_STATIC, subFolder), g_flags);

    attachmentStaticBoxSizer->AddSpacer(20);

    m_delete_attachments = new wxCheckBox(this, wxID_STATIC,
        _("Delete file after import"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_delete_attachments->SetValue(Model_Infotable::instance().GetBoolInfo("ATTACHMENTSDELETE", false));
    m_delete_attachments->SetToolTip(_("Select to delete file after import in attachments archive"));
    attachmentStaticBoxSizer->Add(m_delete_attachments, g_flags);

    m_trash_attachments = new wxCheckBox(this, wxID_STATIC,
        _("When remove attachment, move file instead of delete"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_trash_attachments->SetValue(Model_Infotable::instance().GetBoolInfo("ATTACHMENTSTRASH", false));
    m_trash_attachments->SetToolTip(_("Select to don't delete file when attachment is removed, but instead move it to 'Deleted' subfolder"));
    attachmentStaticBoxSizer->Add(m_trash_attachments, g_flags);
}

void mmOptionAttachmentSettings::OnAttachmentsButton(wxCommandEvent& /*event*/)
{
    wxMenu * attachmentsMenu = new wxMenu;
    wxMenuItem* menuItem = new wxMenuItem(attachmentsMenu, wxID_HIGHEST, _("System documents directory"));
    attachmentsMenu->Append(menuItem);
    menuItem = new wxMenuItem(attachmentsMenu, wxID_HIGHEST + 1, _("Application data directory"));
    attachmentsMenu->Append(menuItem);
    menuItem = new wxMenuItem(attachmentsMenu, wxID_HIGHEST + 2, _("Database file directory"));
    attachmentsMenu->Append(menuItem);
    menuItem = new wxMenuItem(attachmentsMenu, wxID_HIGHEST + 3, _("Dropbox folder"));
    attachmentsMenu->Append(menuItem);
    menuItem = new wxMenuItem(attachmentsMenu, wxID_HIGHEST + 4, _("User Defined"));
    attachmentsMenu->Append(menuItem);

    PopupMenu(attachmentsMenu);
    delete attachmentsMenu;
}

void mmOptionAttachmentSettings::OnAttachmentsMenu(wxCommandEvent& event)
{
    wxTextCtrl* att = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT);
    if (!att) return;
    wxString AttachmentsFolder = mmex::getPathAttachment(att->GetValue());

    int id = event.GetId();
    if (id == wxID_HIGHEST)
        AttachmentsFolder = (ATTACHMENTS_FOLDER_DOCUMENTS);
    else if (id == wxID_HIGHEST + 1)
        AttachmentsFolder = (ATTACHMENTS_FOLDER_APPDATA);
    else if (id == wxID_HIGHEST + 2)
        AttachmentsFolder = (ATTACHMENTS_FOLDER_DATABASE);
    else if (id == wxID_HIGHEST + 3)
        AttachmentsFolder = (ATTACHMENTS_FOLDER_USERPROFILE + "Dropbox");
    else
    {
        wxDirDialog dlg(this
            , _("Choose folder to set as attachments archive")
            , AttachmentsFolder
            , wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST
            );

        if (dlg.ShowModal() != wxID_OK)
            return;

        AttachmentsFolder = dlg.GetPath();
    }
    att->ChangeValue(AttachmentsFolder);

    OnAttachmentsPathChanged(event);
}

void mmOptionAttachmentSettings::OnAttachmentsPathChanged(wxCommandEvent& event)
{
    wxTextCtrl* att = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT);
    if (!att) return;
    wxString AttachmentsFolder = mmex::getPathAttachment(att->GetValue());

    wxStaticText* text = (wxStaticText*) FindWindow(ID_DIALOG_OPTIONS_STATICTEXT_ATTACHMENTSTEXT);
    text->SetLabelText(_("Real path:") + "\n" + AttachmentsFolder);
}

void mmOptionAttachmentSettings::OnAttachmentsSubfolderChanged(wxCommandEvent& event)
{
    Model_Infotable::instance().Set("ATTACHMENTSSUBFOLDER", m_attachments_subfolder->GetValue());
    OnAttachmentsPathChanged(event);
}

void mmOptionAttachmentSettings::SaveSettings()
{
    wxTextCtrl* attTextCtrl = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT);
    wxString attachmentFolder = attTextCtrl->GetValue().Trim();
    Model_Infotable::instance().Set("ATTACHMENTSFOLDER:" + mmPlatformType(), attachmentFolder);
    Model_Infotable::instance().Set("ATTACHMENTSSUBFOLDER", m_attachments_subfolder->GetValue());
    Model_Infotable::instance().Set("ATTACHMENTSDELETE", m_delete_attachments->GetValue());
    Model_Infotable::instance().Set("ATTACHMENTSTRASH", m_trash_attachments->GetValue());

    //Create attachments folder
    wxString attachmentFolderPath = mmex::getPathAttachment(Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:" + mmPlatformType(), ""));
    if (attachmentFolder != wxEmptyString)
    {
        if (!wxDirExists(attachmentFolderPath))
            wxMkdir(attachmentFolderPath);
        if (wxDirExists(attachmentFolderPath))
            mmAttachmentManage::CreateReadmeFile(attachmentFolderPath);
    }
}
