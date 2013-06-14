local function add_header(m_stat)
    mmHTMLTableHeaderCell(_("Month/Year"))
    for y,v in pairs(m_stat[1]) do
        mmHTMLTableHeaderCell(y) 
    end
end
local function get_stats(m_stat)
    local total = 0
    mmHTMLStartTable("50%")
    add_header(m_stat)
    for m, data in pairs(m_stat) do
        mmHTMLStartTableRow()
        mmHTMLTableCellMonth(m)
        for y, val in pairs(data) do
            total = total + val
            mmHTMLTableCellInteger(val)
        end
        mmHTMLEndTableRow()
    end
    mmHTMLEndTable()
    return total
end
-----------------------------------------------------------
--Main function
-----------------------------------------------------------
local function main(m_stat)
    mmHTMLReportHeader(_("Transaction Statistics"))
    mmHTMLhr()
    total = get_stats(m_stat)
    mmHTMLhr()
    mmHTMLaddText(_("Total Transactions: ") .. total)
end

--test table
local m_stat = {}
local y_stat = {[2014]=1,[2013]=2,[2012]=3,[2011]=4,[2010]=5,[2009]=6,[2008]=7,[2007]=8,[2006]=9,[2005]=10}
table.sort(y_stat)
for m = 1, 12, 1 do m_stat[m] = (y_stat) end
--test end
main(m_stat)
