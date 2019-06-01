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

today = date.today()

for x in range(1, 100000):
    account_id = random.choice(all_account)
    payee_id = random.choice(all_payee)
    transcode = random.choice(["Withdrawal", "Deposit"])
    category = random.choice(all_category)
    amount = 1000 * random.random()
    status = random.choice(["V", "R", "N", "F", "D"])
    transdate = today + timedelta(days = random.randint(-1000,10))

    sql = '''INSERT INTO CHECKINGACCOUNT(ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, SUBCATEGID, TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT)
    VALUES(%d, -1, %d, "%s", %f, "%s", "%d", "", %d, %d, "%s", -1, %f)''' % (account_id, payee_id, transcode, amount, status, x, category[0], category[1], transdate, amount)
    curs.execute(sql)

conn.commit()
