local total1 = 0; local total2 = 0; local total3 = 0
local total4 = 0; local total5 = 0; local total6 = 0
local total7 = 0; local total8 = 0; local total9 = 0
local total10 = 0; local total11 = 0; local total12 = 0
local grand_total = 0
function handle_record(record) 
    local year_total = record:get("Jan") + record:get("Feb") + record:get("Mar")
        + record:get("Apr") + record:get("May") + record:get("Jun") + record:get("Jul")
        + record:get("Aug") + record:get("Sep") + record:get("Oct") + record:get("Nov")
        + record:get("Dec")
    grand_total = grand_total + year_total
    total1 = total1 + record:get("Jan")
    total2 = total2 + record:get("Feb")
    total3 = total3 + record:get("Mar")
    total4 = total4 + record:get("Apr")
    total5 = total5 + record:get("May")
    total6 = total6 + record:get("Jun")
    total7 = total7 + record:get("Jul")
    total8 = total8 + record:get("Aug")
    total9 = total9 + record:get("Sep")
    total10 = total10 + record:get("Oct")
    total11 = total11 + record:get("Nov")
    total12 = total12 + record:get("Dec")
    record:set("Jan", math.floor(record:get("Jan")))
    record:set("Feb", math.floor(record:get("Feb")))
    record:set("Mar", math.floor(record:get("Mar")))
    record:set("Apr", math.floor(record:get("Apr")))
    record:set("May", math.floor(record:get("May")))
    record:set("Jun", math.floor(record:get("Jun")))
    record:set("Jul", math.floor(record:get("Jul")))
    record:set("Aug", math.floor(record:get("Aug")))
    record:set("Sep", math.floor(record:get("Sep")))
    record:set("Oct", math.floor(record:get("Oct")))
    record:set("Nov", math.floor(record:get("Nov")))
    record:set("Dec", math.floor(record:get("Dec")))
    record:set("YEAR_TOTAL", year_total)
end

function complete(result)
    result:set("TOTAL_Jan", total1);
    result:set("TOTAL_Feb", total2);
    result:set("TOTAL_Mar", total3);
    result:set("TOTAL_Apr", total4);
    result:set("TOTAL_May", total5);
    result:set("TOTAL_Jun", total6);
    result:set("TOTAL_Jul", total7);
    result:set("TOTAL_Aug", total8);
    result:set("TOTAL_Sep", total9);
    result:set("TOTAL_Oct", total10);
    result:set("TOTAL_Nov", total11);
    result:set("TOTAL_Dec", total12);
    result:set("GRAND_TOTAL", grand_total);
end
