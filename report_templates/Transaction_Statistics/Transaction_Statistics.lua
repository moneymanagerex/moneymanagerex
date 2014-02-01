local totals = {0,0,0,0,0,0,0,0,0,0,0,0}
local grand_total = 0; local count = 0;
local months = {"Jan", "Feb", "Mar" , "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}

function handle_record(record)
    local year_total = 0
    for i=1,12 do
        year_total = year_total +  record:get(months[i]);
        record:set(months[i], math.floor(record:get(months[i])));
        totals[i] = totals[i] + record:get(months[i]);
    end
    count = count +1;
    record:set("YEAR_TOTAL", year_total)
    grand_total = grand_total + year_total;
end

function complete(result)
    for i=1,12 do
         result:set("AVERAGE_" .. months[i], math.floor(totals[i]/count));
    end

    result:set("GRAND_TOTAL", grand_total);
end
