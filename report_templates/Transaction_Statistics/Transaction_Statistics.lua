local total1 = 0; local total2 = 0; local total3 = 0
local total4 = 0; local total5 = 0; local total6 = 0
local total7 = 0; local total8 = 0; local total9 = 0
local total10 = 0; local total11 = 0; local total12 = 0
local grand_total = 0
local count1 = 0; local count2 = 0; local count3 = 0
local count4 = 0; local count5 = 0; local count6 = 0
local count7 = 0; local count8 = 0; local count9 = 0
local count10 = 0; local count11 = 0; local count12 = 0

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
    local month_count = 0
    if math.floor(record:get("Jan")) > 0 then
        count1 = count1 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("Feb")) > 0 then
        count2 = count2 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("Mar")) > 0 then
        count3 = count3 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("Apr")) > 0 then
        count4 = count4 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("May")) > 0 then
        count5 = count5 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("Jun")) > 0 then
        count6= count6 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("Jul")) > 0 then
        count7 = count7 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("Aug")) > 0 then
        count8 = count8 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("Sep")) > 0 then
        count9 = count9 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("Oct")) > 0 then
        count10 = count10 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("Nov")) > 0 then
        count11 = count11 + 1
        month_count = month_count + 1
    end
    if math.floor(record:get("Dec")) > 0 then
        count12 = count12 + 1
        month_count = month_count + 1
    end
    local month_average = 0
    if month_count > 0 then
        month_average = year_total / month_count
    end
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
    record:set("MONTH_AVERAGE", math.floor(month_average))
end

function complete(result)
    local average1 = 0; local average2 = 0; local average3 = 0
    local average4 = 0; local average5 = 0; local average6 = 0
    local average7 = 0; local average8 = 0; local average9 = 0
    local average10 = 0; local average11 = 0; local average12 = 0
    if count1 > 0 then
        average1 = total1 / count1
    end
    if count2 > 0 then
        average2 = total2 / count2
    end
    if count3 > 0 then
        average3 = total3 / count3
    end
    if count4 > 0 then
        average4 = total4 / count4
    end
    if count5 > 0 then
        average5 = total5 / count5
    end
    if count6 > 0 then
        average6 = total6 / count6
    end
    if count7 > 0 then
        average7 = total7 / count7
    end
    if count8 > 0 then
        average8 = total8 / count8
    end
    if count9 > 0 then
        average9= total9 / count9
    end
    if count10 > 0 then
        average10 = total10 / count10
    end
    if count11 > 0 then
        average11 = total11 / count11
    end
    if count12 > 0 then
        average12 = total12 / count12
    end
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
    result:set("AVERAGE_Jan", math.floor(average1));
    result:set("AVERAGE_Feb", math.floor(average2));
    result:set("AVERAGE_Mar", math.floor(average3));
    result:set("AVERAGE_Apr", math.floor(average4));
    result:set("AVERAGE_May", math.floor(average5));
    result:set("AVERAGE_Jun", math.floor(average6));
    result:set("AVERAGE_Jul", math.floor(average7));
    result:set("AVERAGE_Aug", math.floor(average8));
    result:set("AVERAGE_Sep", math.floor(average9));
    result:set("AVERAGE_Oct", math.floor(average10));
    result:set("AVERAGE_Nov", math.floor(average11));
    result:set("AVERAGE_Dec", math.floor(average12));
end
