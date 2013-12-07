local function add_header(header)
    mmHTMLStartTableRow()
    mmHTMLTableHeaderCell(_("Month/Year"))
    for y in pairs(header) do
        mmHTMLTableHeaderCell(header[y]) 
    end
    mmHTMLEndTableRow()
end
local function add_footer(sub_totals)
    mmHTMLStartTableRow()
    mmHTMLTableHeaderCell(_("Sub-Totals:"))
    for y, v in pairs(sub_totals) do
        mmHTMLTableHeaderCell(v) 
    end
    mmHTMLEndTableRow()
end
local function get_stats(m_stat)
    local total = 0
    local sub_total = {}
    mmHTMLStartTable("50%")
    add_header(m_stat.header)
    for m, data in pairs(m_stat.data) do
        mmHTMLStartTableRow()
        mmHTMLTableCellMonth(m)
        for i, val in pairs(data) do
            total = total + val
            if sub_total[m_stat.header[i]] == nil then sub_total[m_stat.header[i]] = 0 end
            sub_total[m_stat.header[i]] = sub_total[m_stat.header[i]] + val
            mmHTMLTableCellInteger(val)
        end
        mmHTMLEndTableRow()
    end
    add_footer(sub_total)
    mmHTMLEndTable()
    return total
end
-----------------------------------------------------------
--Main function
-----------------------------------------------------------
local function main(m_stat)

    mmHTMLReportHeader(m_stat.title)
    mmHTMLhr()
    total = get_stats(m_stat)
    mmHTMLhr()
    mmHTMLaddText(_("Total Transactions: ") .. total)
end

--test
local json_data = [[
{
    "title": "Transaction Statistics",
    "header": ["2014","2013","2012","2011","2010","2009","2008","2007","2006","2005"],
    "data": {
        "1": ["1","2","3","4","5","6","7","8","9","10"],
        "2": ["11","12","13","14","15","16","17","18","19","20"],
        "3": ["21","22","23","24","25","26","27","28","29","30"],
        "4": ["31","32","33","34","35","36","37","38","39","40"],
        "5": ["41","42","43","44","45","46","47","48","49","50"],
        "6": ["51","52","53","54","55","56","57","58","59","60"],
        "7": ["71","72","73","74","75","76","77","78","79","80"],
        "8": ["81","82","83","84","85","86","87","88","89","90"],
        "9": ["91","92","93","94","95","96","97","98","99","100"],
        "10": ["101","102","103","104","105","106","107","108","109","110"],
        "11": ["111","112","113","114","115","116","117","118","119","120"],
        "12": ["121","122","123","124","125","126","127","128","129","130"]
    }
}
]]
local m_stat = cjson.decode(json_data)
main(m_stat)
--test end
