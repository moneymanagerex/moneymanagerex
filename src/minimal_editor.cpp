/////////////////////////////////////////////////////////////////////////////
// Minimal editor added by Troels K 2008-04-08
// Thanks to geralds for SetLexerXml() - http://wxforum.shadonet.com/viewtopic.php?t=7155

#include "minimal_editor.h"
#include "mmex.h"

enum
{
    margin_id_lineno,
    margin_id_fold,
};

wxIMPLEMENT_DYNAMIC_CLASS(MinimalEditor, wxStyledTextCtrl);

wxBEGIN_EVENT_TABLE(MinimalEditor, wxStyledTextCtrl)
    EVT_STC_MARGINCLICK(wxID_ANY, MinimalEditor::OnMarginClick)
    EVT_STC_CHANGE(wxID_ANY, MinimalEditor::OnText)
wxEND_EVENT_TABLE()

MinimalEditor::MinimalEditor(wxWindow* parent, wxWindowID id)
    : wxStyledTextCtrl(parent, id)
    {
        int font_size = this->GetFont().GetPointSize();
        m_font = wxFont(font_size, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    }
bool MinimalEditor::SetFont(const wxFont& font)
    {
        StyleSetFont(wxSTC_STYLE_DEFAULT, const_cast<wxFont&>(font));
        return wxStyledTextCtrl::SetFont(font);
    }

void MinimalEditor::SetLexerSql()
    {
        StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(75, 75, 75));
        StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(220, 220, 220));
        SetMarginWidth(margin_id_lineno, 32);
        StyleClearAll();
        SetLexer(wxSTC_LEX_SQL);
        StyleSetForeground(wxSTC_SQL_WORD, wxColour(0, 150, 0));
        const wxString sqlwords =
            "asc by delete desc from group having insert into order select set update values where";
        SetKeyWords(0, sqlwords);
    }

void MinimalEditor::SetLexerLua() //https://code.google.com/p/wxamcl/source/browse/trunk/scriptedit.cpp?r=63
{
    wxString luawords =
        "function end if then do else for in return break while local repeat elseif and or not false true nil require";
    wxString luawords2 =
        "wxamcl.echo wxamcl.echowin wxamcl.gag wxamcl.color wxamcl.colorwin wxamcl.colorline wxamcl.colorword wxamcl.colorall wxamcl.scroll \
        wxamcl.getlinenumber wxamcl.execute wxamcl.wait wxamcl.waitfor wxamcl.loadprofile \
        wxamcl.createwindow wxamcl.hidewindow wxamcl.showwindow wxamcl.clearwindow \
        wxamcl.action.new wxamcl.action.get wxamcl.action.delete wxamcl.action.enable wxamcl.action.getpattern wxamcl.action.setpattern \
        wxamcl.action.getact wxamcl.action.setact wxamcl.action.getpriority wxamcl.action.setpriority wxamcl.action.getcolmatch wxamcl.action.setcolmatch \
        wxamcl.action.enableall wxamcl.action.disableall wxamcl.action.getall wxamcl.action.delgroup wxamcl.action.enablegroup \
        wxamcl.alias.new wxamcl.alias.get wxamcl.alias.delete wxamcl.alias.enable wxamcl.alias.getall wxamcl.alias.getaction\
        wxamcl.alias.setaction wxamcl.alias.delgroup wxamcl.alias.enablegroup \
        wxamcl.hk.new wxamcl.hk.delgroup wxamcl.hk.delete wxamcl.hk.enable \
        wxamcl.var.new wxamcl.var.set wxamcl.var.get wxamcl.var.delete wxamcl.var.enable \
        wxamcl.gauge.update wxamcl.gauge.new wxamcl.gauge.setsize wxamcl.gauge.setcolor wxamcl.gauge.createwindow wxamcl.gauge.setlabel \
        wxamcl.timer.new wxamcl.timer.delete wxamcl.timer.start wxamcl.timer.stop wxamcl.timer.enable \
        wxamcl.mxp.enable wxamcl.mxp.echo wxamcl.mxp.echowin \
        wxamcl.list.new wxamcl.list.additem wxamcl.list.clear wxamcl.list.delete wxamcl.list.contains wxamcl.list.delitem wxamcl.list.delitemat \
        wxamcl.gmcp.send \
        wxamcl.db.open wxamcl.db.execute wxamcl.db.columns wxamcl.db.results wxamcl.db.close wxamcl.db.insert \
        wxamcl.draw.text wxamcl.draw.line wxamcl.draw.circle wxamcl.draw.square wxamcl.draw.winsize";
    wxString amcwords =
        "clearwin capstart capend setvar gag help loadprofile func script bscript gagwin connect pwd refreshwin raw capturewin capturenb log htmllog resume test";

    SetLexer(wxSTC_LEX_LUA);

    SetKeyWords(0, luawords);
    SetKeyWords(1, luawords2);
    //SetKeyWords(1, luawords3);
    SetKeyWords(2, amcwords);

    SetMarginWidth(0, 0);
    SetMarginType(0, wxSTC_MARGIN_NUMBER);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour("DARK GREY"));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxLIGHT_GREY);

    // set margin as unused
    SetMarginType(1, wxSTC_MARGIN_SYMBOL);
    SetMarginWidth(1, 0);
    SetMarginSensitive(1, false);

    // set visibility
    SetVisiblePolicy(wxSTC_VISIBLE_STRICT | wxSTC_VISIBLE_SLOP, 1);
    SetXCaretPolicy(wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1);
    SetYCaretPolicy(wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1);

    SetProperty("fold.compact", "0");
    SetProperty("fold.comment", "1");
    SetProperty("fold", "1");
    SetMarginWidth(2, 0);
    SetMarginType(2, wxSTC_MARGIN_SYMBOL);
    SetMarginMask(2, wxSTC_MASK_FOLDERS);
    StyleSetBackground(2, *wxWHITE);
    SetMarginSensitive(2, true);

    SetMarginWidth(0, 36);
    SetMarginWidth(2, 16);
    // markers
    MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS, "WHITE", "BLACK");
    MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS, "WHITE", "BLACK");
    MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE, "WHITE", "BLACK");
    MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUSCONNECTED, "WHITE", "BLACK");
    MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUSCONNECTED, "WHITE", "WHITE");
    MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER, "WHITE", "BLACK");
    MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNER, "WHITE", "BLACK");
    SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED |
        wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);


    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour("DARK GREY"));

    // set spaces and indention
    SetTabWidth(4);
    SetUseTabs(false);
    SetTabIndents(true);
    SetBackSpaceUnIndents(true);
    //Set Lua styles

    StyleSetFont(wxSTC_LUA_DEFAULT, m_font); //TODO;
    StyleSetForeground(wxSTC_LUA_DEFAULT, *wxBLACK);
    StyleSetBackground (wxSTC_LUA_DEFAULT, *wxWHITE);
    StyleSetForeground(wxSTC_LUA_COMMENTLINE, wxColour("FOREST GREEN"));
    StyleSetForeground(wxSTC_LUA_COMMENT, wxColour("FOREST GREEN"));
    StyleSetForeground(wxSTC_LUA_COMMENTDOC, wxColour("FOREST GREEN"));
    StyleSetForeground(wxSTC_LUA_PREPROCESSOR, wxColour("BLUE"));
    //StyleSetFont(wxSTC_LUA_WORD, bfont);
    StyleSetBold(wxSTC_LUA_WORD, true);
    StyleSetForeground(wxSTC_LUA_WORD, wxColour("BLUE"));
    StyleSetFont(wxSTC_LUA_WORD2, m_font);
    StyleSetForeground(wxSTC_LUA_WORD2, wxColour("BLUE"));
    StyleSetForeground(wxSTC_LUA_WORD3, wxColour("MEDIUM BLUE"));
    //StyleSetFont(wxSTC_LUA_WORD3, bfont);
    StyleSetBold(wxSTC_LUA_WORD3, true);
    StyleSetFont(wxSTC_LUA_STRING, m_font);
    StyleSetForeground(wxSTC_LUA_STRING, wxColour("RED"));
    StyleSetFont(wxSTC_LUA_CHARACTER, m_font);
    StyleSetForeground(wxSTC_LUA_CHARACTER, wxColour("RED"));
    StyleSetFont(wxSTC_LUA_LITERALSTRING, m_font);
    StyleSetForeground(wxSTC_LUA_LITERALSTRING, wxColour("ORANGE RED"));
    StyleSetFont(wxSTC_LUA_NUMBER, m_font);
    StyleSetForeground(wxSTC_LUA_NUMBER, wxColour("SIENNA"));
    StyleSetFont(wxSTC_LUA_STRINGEOL, m_font);
    StyleSetForeground(wxSTC_LUA_STRINGEOL, wxColour("SIENNA"));
    StyleSetFont(wxSTC_LUA_OPERATOR, m_font);
    StyleSetForeground(wxSTC_LUA_OPERATOR, wxColour("DARK ORCHID"));
    StyleSetFont(wxSTC_LUA_IDENTIFIER, m_font);
    StyleSetForeground(wxSTC_LUA_IDENTIFIER, wxColour("DARK ORCHID"));
    SetViewWhiteSpace(false);
    SetCurrentPos(0);
}

void MinimalEditor::SetLexerHtml()
{
    static const wxString templatewords = "TMPL_VAR TMPL_LOOP TMPL_IF __FIRST__ __LAST_ __COUNT__ __COUNTER__ __TOTAL__ __ODD__ __EVEN__ __INNER__ ";
    SetLexer(wxSTC_LEX_HTML);
    SetKeyWords(0, templatewords);
    SetMarginWidth(margin_id_lineno, 32);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(75, 75, 75));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(220, 220, 220));
    SetWrapMode(wxSTC_WRAP_WORD);
    StyleClearAll();
    StyleSetForeground(wxSTC_H_DOUBLESTRING, *wxRED);
    StyleSetForeground(wxSTC_H_SINGLESTRING, *wxRED);
    StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
    StyleSetForeground(wxSTC_H_TAG, wxColour(0, 150, 0));
    StyleSetForeground(wxSTC_H_TAGUNKNOWN, wxColour(0, 150, 0));
    StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour(0, 0, 150));
    StyleSetForeground(wxSTC_H_ATTRIBUTEUNKNOWN, wxColour(0, 0, 150));
    StyleSetForeground(wxSTC_H_COMMENT, wxColour(150, 150, 150));
}

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
    event.Skip();
}
