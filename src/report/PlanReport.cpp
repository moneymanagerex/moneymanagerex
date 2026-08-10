/*******************************************************
 Copyright (C) 2026

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

#include "PlanReport.h"

#include <vector>
#include "base/_defs.h"
#include "model/AccountModel.h"
#include "model/BudgetPeriodModel.h"
#include "model/BudgetSegmentModel.h"
#include "model/PlanAssumptionModel.h"
#include "model/PlanAssumptionGroupModel.h"
#include "model/PlanEngine.h"
#include "model/PlanGroupModel.h"
#include "model/PlanItemModel.h"
#include "model/CategoryModel.h"
#include "model/CurrencyModel.h"
#include "util/_util.h"
#include "htmlbuilder.h"
#include <functional>

namespace
{
    // Marker appended to any figure that is derived from an assumption, so a
    // guess is never mistaken for a fact.
    const char* ASSUMED_MARK = " ~";

    wxString status_label(const PlanStatus& s)
    {
        return wxGetTranslation(s.name());
    }

    // mmHTMLBuilder has no paragraph helper; this keeps the call sites readable.
    void addParagraph(mmHTMLBuilder& hb, const wxString& text)
    {
        hb.addText("<p>" + text + "</p>");
    }

    // Units are a count, not money: show enough precision for a fractional
    // share without padding a whole number with meaningless zeros.
    wxString format_units(double units)
    {
        wxString s = wxString::FromDouble(units, 4);
        while (s.EndsWith("0")) s.RemoveLast();
        if (s.EndsWith(".") || s.EndsWith(",")) s.RemoveLast();
        return s;
    }

    // A value read together with the unit it is measured in: a price as money,
    // a rate as a percentage, anything else with the unit spelled out.
    wxString format_assumption_value(const PlanAssumptionData& a, const wxString& group_unit)
    {
        const wxString unit = group_unit.IsEmpty() ? a.unit_or_default() : group_unit;

        if (a.m_kind.is_rate())
            return wxString::Format("%s %s",
                wxString::FromDouble(a.as_rate() * 100.0, 2),
                unit.IsEmpty() ? "%" : unit);

        if (unit.IsEmpty())
            return CurrencyModel::instance().toCurrency(a.m_value);

        return wxString::Format("%s %s", wxString::FromDouble(a.m_value, 4), unit);
    }

    wxString date_or_undated(const wxString& iso)
    {
        return iso.IsEmpty() ? _t("Undated") : mmGetDateTimeForDisplay(iso);
    }
}

// =====================================================================
// Long-term plan
// =====================================================================

PlanLongTermReport::PlanLongTermReport() :
    ReportBase(_n("Long-Term Plan"))
{
    setReportParameters(REPORT_ID::PlanLongTerm);
}

PlanLongTermReport::~PlanLongTermReport()
{}

wxString PlanLongTermReport::getHTMLText()
{
    const mmDate as_of = mmDate::today();

    const PlanSummary summary = PlanEngine::build_summary(as_of);
    PlanItemModel& pim = PlanItemModel::instance();
    PlanGroupModel& pgm = PlanGroupModel::instance();

    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getTitle(), 1, false);
    hb.addDateNow();

    // ---- Current assets -------------------------------------------------
    hb.addDivContainer("shadow");
    {
        hb.addHeader(2, _t("Current assets"));
        hb.startTable();
        {
            hb.startTbody();
            {
                hb.addTableRow(_t("Cash"), summary.cash_assets);
                hb.addTableRow(_t("Investments"), summary.investment_assets);
                hb.addTableRowBold(_t("Total assets"), summary.total_assets);
            }
            hb.endTbody();
        }
        hb.endTable();
    }
    hb.endDiv();

    // ---- Assumptions ----------------------------------------------------
    // Stated up front: every figure below inherits whatever is wrong here.
    PlanAssumptionModel& pam = PlanAssumptionModel::instance();
    PlanAssumptionGroupModel& pagm = PlanAssumptionGroupModel::instance();

    const PlanAssumptionModel::DataA assumptions = pam.find_active_a();
    const PlanAssumptionGroupModel::DataA groups = pagm.find_active_a();

    hb.addDivContainer("shadow");
    {
        hb.addHeader(2, _t("Assumptions"));
        if (assumptions.empty() && groups.empty()) {
            addParagraph(hb, _t(
                "No assumptions are defined. Values based on share prices or tax "
                "rates fall back to the amounts stored on each plan item."));
        }
        else {
            hb.startTable();
            {
                hb.startThead();
                {
                    hb.startTableRow();
                    {
                        hb.addTableHeaderCell(_t("Assumption"));
                        hb.addTableHeaderCell(_t("Type"));
                        hb.addTableHeaderCell(_t("Applies to"));
                        hb.addTableHeaderCell(_t("Value"), "text-right");
                        hb.addTableHeaderCell(_t("Used by"), "text-right");
                    }
                    hb.endTableRow();
                }
                hb.endThead();

                hb.startTbody();

                // A group is shown with the member currently in force, and its
                // alternatives underneath, so it is clear both what the plan is
                // using and what else it could be switched to.
                for (const auto& g : groups) {
                    const int used = pagm.count_dependents(g.m_id);
                    const int64 active_id = pagm.get_active_assumption_id(g.m_id);
                    const PlanAssumptionData* active_n =
                        (active_id > 0) ? pam.get_idN_data_n(active_id) : nullptr;

                    hb.startTableRow("bold");
                    {
                        hb.addTableCell(g.m_name);
                        hb.addTableCell(wxGetTranslation(g.m_kind.name()));
                        hb.addTableCell(g.m_scope_key.IsEmpty() ? "-" : g.m_scope_key);
                        hb.addTableCell(active_n
                            ? format_assumption_value(*active_n, g.m_unit)
                            : _t("none selected"), true);
                        hb.addTableCell(wxString::Format("%d", used), true);
                    }
                    hb.endTableRow();

                    for (const auto& a : pam.find_group_a(g.m_id)) {
                        const bool is_active = (a.m_id == active_id);
                        hb.startTableRow();
                        {
                            hb.addTableCell("&nbsp;&nbsp;" + a.m_name +
                                (is_active ? " " + _t("(active)") : ""));
                            hb.addEmptyTableCell(2);
                            hb.addTableCell(format_assumption_value(a, g.m_unit), true);
                            hb.addEmptyTableCell();
                        }
                        hb.endTableRow();
                    }
                }

                // Standalone assumptions are pinned directly by items.
                for (const auto& a : assumptions) {
                    if (a.is_grouped())
                        continue;

                    const std::size_t used = pam.count_dependents(a.m_id);
                    hb.startTableRow();
                    {
                        hb.addTableCell(a.m_name);
                        hb.addTableCell(wxGetTranslation(a.m_kind.name()));
                        hb.addTableCell(a.m_scope_key.IsEmpty() ? "-" : a.m_scope_key);
                        hb.addTableCell(format_assumption_value(a, wxEmptyString), true);
                        hb.addTableCell(wxString::Format("%zu", used), true);
                    }
                    hb.endTableRow();
                }
                hb.endTbody();
            }
            hb.endTable();

            if (summary.expected_income > 0.0) {
                addParagraph(hb, wxString::Format(
                    _t("%s of expected income is calculated from these assumptions."),
                    wxString::Format("%.0f%%", summary.assumption_exposure() * 100.0)));
            }
        }
    }
    hb.endDiv();

    // ---- Expected income ------------------------------------------------
    const PlanItemModel::DataA future =
        pim.find_between_a(as_of.isoDate(), "", true);

    hb.addDivContainer("shadow");
    {
        hb.addHeader(2, _t("Expected income"));
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_t("Date"));
                    hb.addTableHeaderCell(_t("Item"));
                    hb.addTableHeaderCell(_t("Units"), "text-right");
                    hb.addTableHeaderCell(_t("Unit price"), "text-right");
                    hb.addTableHeaderCell(_t("Net"), "text-right");
                }
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            double income_total = 0.0;
            for (const auto& item : future) {
                if (!item.m_kind.is_income() || !item.m_status.is_active_plan())
                    continue;

                const double net = pim.net_amount_base(item);
                income_total += net;

                hb.startTableRow();
                {
                    // An undated row says so rather than showing an empty cell
                    // that could be read as a missing value.
                    if (item.m_target_date.IsEmpty())
                        hb.addTableCell(_t("Undated"));
                    else
                        hb.addTableCellDate(item.m_target_date);

                    // Flag rows whose value is only as good as an assumption.
                    hb.addTableCell(item.is_unit_based() || item.is_assumption_based()
                        ? item.m_name + ASSUMED_MARK
                        : item.m_name);
                    if (item.is_unit_based()) {
                        hb.addTableCell(format_units(item.m_units), true);
                        hb.addMoneyCell(pim.resolve_unit_price(item));
                    }
                    else {
                        hb.addEmptyTableCell(2);
                    }
                    hb.addMoneyCell(net);
                }
                hb.endTableRow();
            }
            hb.endTbody();

            hb.startTfoot();
            hb.addMoneyTotalRow(_t("Total expected income"), 5, { income_total });
            hb.endTfoot();
        }
        hb.endTable();
    }
    hb.endDiv();

    // ---- Planned expenses, grouped --------------------------------------
    hb.addDivContainer("shadow");
    {
        hb.addHeader(2, _t("Planned expenses"));
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_t("Group / Item"));
                    hb.addTableHeaderCell(_t("Target date"));
                    hb.addTableHeaderCell(_t("Status"));
                    hb.addTableHeaderCell(_t("Amount"), "text-right");
                }
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();

            // Groups nest to any depth, so each level is rendered in place and
            // indented, rather than flattening a whole project into one list.
            std::function<double(int64, int)> render_group =
                [&](int64 group_id, int depth) -> double {
                    if (depth > 32)
                        return 0.0;

                    const PlanGroupData* g_n = pgm.get_idN_data_n(group_id);
                    if (!g_n)
                        return 0.0;

                    // wxString has no Repeat helper, so build the indent by hand.
                    wxString indent;
                    for (int i = 0; i < depth; ++i)
                        indent += "&nbsp;&nbsp;";

                    hb.startTableRow(depth == 0 ? "bold" : "");
                    {
                        hb.addTableCell(indent + g_n->m_name);
                        hb.addTableCell(date_or_undated(g_n->m_target_date));
                        hb.addTableCell(status_label(g_n->m_status));
                        hb.addEmptyTableCell();
                    }
                    hb.endTableRow();

                    double total = 0.0;

                    for (const auto& item : pim.find_group_a(group_id)) {
                        if (item.m_kind.is_income() || !item.m_status.is_active_plan())
                            continue;

                        const double amt = pim.net_amount_base(item);
                        total += amt;

                        wxString label = indent + "&nbsp;&nbsp;" + item.m_name;
                        if (item.is_unit_based() || item.is_assumption_based())
                            label += ASSUMED_MARK;
                        if (item.m_category_id > 0)
                            label += " <small>(" +
                                CategoryModel::instance().get_id_fullname(item.m_category_id, ":") +
                                ")</small>";

                        hb.startTableRow();
                        {
                            hb.addTableCell(label);
                            hb.addTableCell(date_or_undated(item.m_target_date));
                            hb.addTableCell(status_label(item.m_status));
                            hb.addMoneyCell(amt);
                        }
                        hb.endTableRow();
                    }

                    for (const auto& child : pgm.find_children_a(group_id))
                        total += render_group(child.m_id, depth + 1);

                    // A subtotal is only worth the row when the group actually
                    // contains something.
                    if (total != 0.0) {
                        hb.startTableRow(depth == 0 ? "bold" : "");
                        {
                            hb.addTableCell(indent + _t("Subtotal") + " - " + g_n->m_name);
                            hb.addEmptyTableCell(2);
                            hb.addMoneyCell(total);
                        }
                        hb.endTableRow();
                    }

                    return total;
                };

            for (const auto& group : pgm.find_root_a())
                render_group(group.m_id, 0);

            // Expenses that belong to no group would otherwise be invisible here
            // while still counting towards the totals below.
            bool ungrouped_header = false;
            double ungrouped_total = 0.0;
            for (const auto& item : pim.find_group_a(-1)) {
                if (item.m_kind.is_income() || !item.m_status.is_active_plan())
                    continue;

                if (!ungrouped_header) {
                    hb.startTableRow("bold");
                    {
                        hb.addTableCell(_t("Ungrouped"));
                        hb.addEmptyTableCell(3);
                    }
                    hb.endTableRow();
                    ungrouped_header = true;
                }

                const double amt = pim.net_amount_base(item);
                ungrouped_total += amt;

                hb.startTableRow();
                {
                    hb.addTableCell("&nbsp;&nbsp;" + item.m_name);
                    hb.addTableCell(date_or_undated(item.m_target_date));
                    hb.addTableCell(status_label(item.m_status));
                    hb.addMoneyCell(amt);
                }
                hb.endTableRow();
            }
            if (ungrouped_total != 0.0) {
                hb.startTableRow("bold");
                {
                    hb.addTableCell(_t("Subtotal") + " - " + _t("Ungrouped"));
                    hb.addEmptyTableCell(2);
                    hb.addMoneyCell(ungrouped_total);
                }
                hb.endTableRow();
            }

            hb.endTbody();

            hb.startTfoot();
            hb.addMoneyTotalRow(_t("Committed"), 4, { summary.committed_expense });
            hb.addMoneyTotalRow(_t("Wishlist (not committed)"), 4, { summary.wishlist_expense });
            hb.endTfoot();
        }
        hb.endTable();
    }
    hb.endDiv();

    // ---- Free assets ----------------------------------------------------
    hb.addDivContainer("shadow");
    {
        hb.addHeader(2, _t("Free assets"));
        hb.startTable();
        {
            hb.startTbody();
            {
                hb.addTableRow(_t("Total assets"), summary.total_assets);
                hb.addTableRow(_t("Expected income"), summary.expected_income);
                hb.addTableRow(_t("Less committed expenses"), -summary.committed_expense);
                hb.addTableRowBold(_t("Free assets"), summary.free_assets());
                hb.addTableRow(_t("If the wishlist is also acted on"),
                    summary.free_assets_after_wishlist());
            }
            hb.endTbody();
        }
        hb.endTable();
    }
    hb.endDiv();

    // ---- Sensitivity ----------------------------------------------------
    // The plan is a forecast. This shows how much it moves when the guesses
    // behind it are wrong, which is the difference between a plan and a hope.
    if (summary.assumption_based_income != 0.0 || summary.investment_assets != 0.0) {
        const std::vector<double> shifts = { -30.0, -20.0, -10.0, 10.0, 20.0 };
        const std::vector<PlanSensitivity> sens =
            PlanEngine::build_sensitivity_a(as_of, shifts);

        hb.addDivContainer("shadow");
        {
            hb.addHeader(2, _t("If the assumptions are wrong"));
            hb.startTable();
            {
                hb.startThead();
                {
                    hb.startTableRow();
                    {
                        hb.addTableHeaderCell(_t("Scenario"));
                        hb.addTableHeaderCell(_t("Expected income"), "text-right");
                        hb.addTableHeaderCell(_t("Free assets"), "text-right");
                        hb.addTableHeaderCell(_t("Change"), "text-right");
                    }
                    hb.endTableRow();
                }
                hb.endThead();

                hb.startTbody();
                for (const auto& s : sens) {
                    hb.startTableRow();
                    {
                        hb.addTableCell(wxString::Format(
                            _t("Assumed values %+.0f%%"), s.shift_pct));
                        hb.addMoneyCell(s.expected_income);
                        hb.addMoneyCell(s.free_assets);
                        hb.addMoneyCell(s.delta);
                    }
                    hb.endTableRow();
                }
                hb.endTbody();
            }
            hb.endTable();
            addParagraph(hb, _t(
                "Values marked ~ are calculated from an assumption rather than "
                "from a recorded amount."));
        }
        hb.endDiv();
    }

    hb.end();
    return hb.getHTMLText();
}

// =====================================================================
// Segment cash flow
// =====================================================================

PlanCashFlowReport::PlanCashFlowReport() :
    ReportBase(_n("Planned Cash Flow"))
{
    setReportParameters(REPORT_ID::PlanCashFlow);
}

PlanCashFlowReport::~PlanCashFlowReport()
{}

wxString PlanCashFlowReport::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getTitle(), 1, false);
    hb.addDateNow();

    // m_date_selection carries the budget period chosen in the report toolbar.
    const int64 bp_id = m_date_selection;
    const wxString bp_name = BudgetPeriodModel::instance().get_id_name_n(bp_id);

    if (bp_name.IsEmpty()) {
        addParagraph(hb, _t("Select a budget period to project its cash flow."));
        hb.end();
        return hb.getHTMLText();
    }

    const int months = (m_forward_months > 0) ? m_forward_months : 12;
    const std::vector<PlanPeriod> timeline =
        PlanEngine::build_timeline(bp_id, mmDate::today(), months);

    hb.addDivContainer("shadow");
    {
        hb.addHeader(2, bp_name);

        if (timeline.empty()) {
            addParagraph(hb, _t("This budget period has no entries to project."));
        }
        else {
            hb.startTable();
            {
                hb.startThead();
                {
                    hb.startTableRow();
                    {
                        hb.addTableHeaderCell(_t("Period"));
                        hb.addTableHeaderCell(_t("Income"), "text-right");
                        hb.addTableHeaderCell(_t("Expenses"), "text-right");
                        hb.addTableHeaderCell(_t("Net"), "text-right");
                        hb.addTableHeaderCell(_t("Balance"), "text-right");
                    }
                    hb.endTableRow();
                }
                hb.endThead();

                hb.startTbody();
                for (const auto& p : timeline) {
                    // Draw attention to a projected overdraft; that is the whole
                    // reason for planning in halves.
                    const bool negative = p.closing_balance < 0.0;
                    if (negative) hb.startTableRow("bold");
                    else          hb.startTableRow();
                    {
                        wxString label = p.label;
                        if (p.has_unresolved)
                            label += " (!)";
                        else if (p.has_estimates)
                            label += " ~";
                        hb.addTableCell(label);
                        hb.addMoneyCell(p.income + p.plan_income);
                        hb.addMoneyCell(p.expense + p.plan_expense);
                        hb.addMoneyCell(p.net());
                        hb.addMoneyCell(p.closing_balance);
                    }
                    hb.endTableRow();
                }
                hb.endTbody();
            }
            hb.endTable();

            addParagraph(hb, _t(
                "~ marks a period containing estimated or automatically derived "
                "amounts. (!) marks an automatic amount that could not be resolved."));
        }
    }
    hb.endDiv();

    hb.end();
    return hb.getHTMLText();
}
