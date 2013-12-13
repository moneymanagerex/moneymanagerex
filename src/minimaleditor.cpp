
/////////////////////////////////////////////////////////////////////////////
// Minimal editor added by Troels K 2008-04-08
// Thanks to geralds for SetLexerXml() - http://wxforum.shadonet.com/viewtopic.php?t=7155

class MinimalEditor : public wxStyledTextCtrl
{
    enum
    {
        margin_id_lineno,
        margin_id_fold,
    };

public:
    MinimalEditor(wxWindow* parent, wxWindowID id = wxID_ANY) : wxStyledTextCtrl(parent, id)
    {
        SetLexerXml();

        SetProperty(wxT("fold"), wxT("1"));
        SetProperty(wxT("fold.comment"), wxT("1"));
        SetProperty(wxT("fold.compact"), wxT("1"));
        SetProperty(wxT("fold.preprocessor"), wxT("1"));
        SetProperty(wxT("fold.html"), wxT("1"));
        SetProperty(wxT("fold.html.preprocessor"), wxT("1"));

        SetMarginType(margin_id_lineno, wxSTC_MARGIN_NUMBER);
        SetMarginWidth(margin_id_lineno, 32);

        MarkerDefine(wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_BOXPLUS, wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_BOXMINUS,  wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_VLINE,     wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_BOXPLUSCONNECTED, wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUSCONNECTED, wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER,     wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_LCORNER,     wxT("WHITE"), wxT("BLACK"));

        SetMarginMask(margin_id_fold, wxSTC_MASK_FOLDERS);
        SetMarginWidth(margin_id_fold, 32);
        SetMarginSensitive(margin_id_fold, true);

        SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

        SetTabWidth(4);
        SetUseTabs(false);
        SetWrapMode(wxSTC_WRAP_WORD);
        SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_END);
    }
    virtual bool SetFont(const wxFont& font)
    {
        StyleSetFont(wxSTC_STYLE_DEFAULT, (wxFont&)font);
        return wxStyledTextCtrl::SetFont(font);
    }
    void SetLexerXml()
    {
        SetLexer(wxSTC_LEX_XML);
        StyleSetForeground(wxSTC_H_DEFAULT, *wxBLACK);
        StyleSetForeground(wxSTC_H_TAG, *wxBLUE);
        StyleSetForeground(wxSTC_H_TAGUNKNOWN, *wxBLUE);
        StyleSetForeground(wxSTC_H_ATTRIBUTE, *wxRED);
        StyleSetForeground(wxSTC_H_ATTRIBUTEUNKNOWN, *wxRED);
        StyleSetBold(wxSTC_H_ATTRIBUTEUNKNOWN, true);
        StyleSetForeground(wxSTC_H_NUMBER, *wxBLACK);
        StyleSetForeground(wxSTC_H_DOUBLESTRING, *wxBLACK);
        StyleSetForeground(wxSTC_H_SINGLESTRING, *wxBLACK);
        StyleSetForeground(wxSTC_H_OTHER, *wxBLUE);
        StyleSetForeground(wxSTC_H_COMMENT, wxTheColourDatabase->Find(wxT("GREY")));
        StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
        StyleSetBold(wxSTC_H_ENTITY, true);
        StyleSetForeground(wxSTC_H_TAGEND, *wxBLUE);
        StyleSetForeground(wxSTC_H_XMLSTART, *wxBLUE);
        StyleSetForeground(wxSTC_H_XMLEND, *wxBLUE);
        StyleSetForeground(wxSTC_H_CDATA, *wxRED);
    }
protected:
    void OnMarginClick(wxStyledTextEvent&);
    void OnText(wxStyledTextEvent&);
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MinimalEditor, wxStyledTextCtrl)
    EVT_STC_MARGINCLICK(wxID_ANY, MinimalEditor::OnMarginClick)
    EVT_STC_CHANGE(wxID_ANY, MinimalEditor::OnText)
END_EVENT_TABLE()

void MinimalEditor::OnMarginClick(wxStyledTextEvent &event)
{
    if (event.GetMargin() == margin_id_fold)
    {
        int lineClick = LineFromPosition(event.GetPosition());
        int levelClick = GetFoldLevel(lineClick);
        if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0)
        {
            ToggleFold(lineClick);
        }
    }
}

void MinimalEditor::OnText(wxStyledTextEvent& event)
{
    wxLogDebug(wxT("Modified"));
    event.Skip();
}

