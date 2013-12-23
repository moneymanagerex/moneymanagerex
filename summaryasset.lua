local total_balance = 0
function handle_record(record) 
    record:set("xxxx", "YYYYY");
    total_balance = total_balance + record:get("VALUE");
end

function complete(result)
    result:set("ASSET_BALANCE", total_balance);
end
