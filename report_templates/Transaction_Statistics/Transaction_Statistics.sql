select strftime('%Y', TRANSDATE) as YEAR
    , total('01' = strftime('%m', TRANSDATE)) as Jan
    , total('02' = strftime('%m', TRANSDATE)) as Feb
    , total('03' = strftime('%m', TRANSDATE)) as Mar
    , total('04' = strftime('%m', TRANSDATE)) as Apr
    , total('05' = strftime('%m', TRANSDATE)) as May
    , total('06' = strftime('%m', TRANSDATE)) as Jun
    , total('07' = strftime('%m', TRANSDATE)) as Jul
    , total('08' = strftime('%m', TRANSDATE)) as Aug
    , total('09' = strftime('%m', TRANSDATE)) as Sep
    , total('10' = strftime('%m', TRANSDATE)) as Oct
    , total('11' = strftime('%m', TRANSDATE)) as Nov
    , total('12' = strftime('%m', TRANSDATE)) as Dec
from CHECKINGACCOUNT_V1 
group by year
order by year desc;
