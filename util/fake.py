import sys
import random
import sqlite3
from datetime import date, timedelta

conn = sqlite3.connect(sys.argv[1])
curs = conn.cursor()

curs.execute("select * from ACCOUNTLIST where ACCOUNTTYPE != 'Investment'")
all_account = [account[0] for account in curs.fetchall()]

curs.execute("select * from PAYEE")
all_payee = [payee[0] for payee in curs.fetchall()]

curs.execute("select c.CATEGID, ifnull(s.SUBCATEGID, -1) from CATEGORY as c left join SUBCATEGORY as s ON (c.CATEGID = s.CATEGID)")
all_category = [(c[0], c[1]) for c in curs.fetchall()]

all_type = ("Withdrawal", "Deposit", "Transfer")
all_status = ("V", "R", "N", "F", "D")

today = date.today()

for x in range(1, 100000):
    account_id = all_account[random.randint(0, len(all_account) -1)]
    payee_id = all_payee[random.randint(0, len(all_payee) -1)]
    transcode = all_type[random.randint(0, len(all_type) - 2)]
    category = all_category[random.randint(0, len(all_category) -1)]
    amount = 1000 * random.random()
    type = all_status[random.randint(0, len(all_status) -1)]
    transdate = today + timedelta(days = random.randint(-1000,10)) 
    
    sql = '''INSERT INTO CHECKINGACCOUNT(ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, SUBCATEGID, TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT)
    VALUES(%d, -1, %d, "%s", %f, "%s", "%d", "", %d, %d, "%s", -1, %f)''' % (account_id, payee_id, transcode, amount, type, x, category[0], category[1], transdate, amount)
    curs.execute(sql)
    
conn.commit()
