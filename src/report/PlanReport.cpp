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
#include "model/PlanEngine.h"
#include "model/PlanGroupModel.h"
#include "model/PlanItemModel.h"
#include "htmlbuilder.h"

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
    const PlanAssumptionModel::DataA assumptions =
        PlanAssumptionModel::instance().find_active_a();

    hb.addDivContainer("shadow");
    {
        hb.addHeader(2, _t("Assumptions"));
        if (assumptions.empty()) {
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
                for (const auto& a : assumptions) {
                    const std::size_t used =
                        PlanAssumptionModel::instance().count_dependents(a.m_id);
                    hb.startTableRow();
                    {
                        hb.addTableCell(a.m_name);
                        hb.addTableCell(wxGetTranslation(a.m_kind.name()));
                        hb.addTableCell(a.m_scope_key.IsEmpty() ? "-" : a.m_scope_key);
                        if (a.m_kind.is_rate())
                            hb.addTableCell(wxString::Format("%.2f %%", a.as_rate() * 100.0), true);
                        else
                            hb.addMoneyCell(a.m_value);
                        hb.addTableCell(wxString::Format("%zu", used), true);
                    }
                    hb.endTableRow();
                }
                hb.endTbody();
            }
            hb.endTable();

            if (summary.expected_income > 0.0) {
                addParagraph(hb, wxString::Format(
                    _t("%.0f%% of expected income is calculated from these assumptions."),
                    summary.assumption_exposure() * 100.0));
            }
        }
    }
    hb.endDiv();

    // ---- Expected income ------------------------------------------------
    const PlanItemModel::DataA future =
        pim.find_between_a(as_of.isoDate(), "");

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
                    hb.addTableCellDate(item.m_target_date);
                    // Flag rows whose value is only as good as an assumption.
                    hb.addTableCell(item.is_unit_based() || item.is_assumption_based()
                        ? item.m_name + ASSUMED_MARK
                        : item.m_name);
                    if (item.is_unit_based()) {
                        hb.addTableCell(wxString::Format("%.3f", item.m_units), true);
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
            for (const auto& group : pgm.find_root_a()) {
                double group_total = 0.0;

                // Include nested groups so a trip with sub-groups totals once.
                const std::vector<int64> subtree = pgm.find_subtree_id_a(group.m_id);

                hb.startTableRow("bold");
                {
                    hb.addTableCell(group.m_name);
                    hb.addTableCellDate(group.m_target_date);
                    hb.addTableCell(status_label(group.m_status));
                    hb.addEmptyTableCell();
                }
                hb.endTableRow();

                for (int64 gid : subtree) {
                    for (const auto& item : pim.find_group_a(gid)) {
                        if (item.m_kind.is_income() || !item.m_status.is_active_plan())
                            continue;

                        const double amt = pim.net_amount_base(item);
                        group_total += amt;

                        hb.startTableRow();
                        {
                            hb.addTableCell("&nbsp;&nbsp;" + item.m_name);
                            hb.addTableCellDate(item.m_target_date);
                            hb.addTableCell(status_label(item.m_status));
                            hb.addMoneyCell(amt);
                        }
                        hb.endTableRow();
                    }
                }

                hb.startTableRow();
                {
                    hb.addTableCell(_t("Subtotal") + " - " + group.m_name);
                    hb.addEmptyTableCell(2);
                    hb.addMoneyCell(group_total);
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
