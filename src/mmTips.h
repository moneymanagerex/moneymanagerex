/*******************************************************
Copyright (C) 2013 Nikolay Akimov

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
#pragma once

static const wxString TIPS [] =
{
    wxTRANSLATE("Recommendation: Always backup your .mmb database file regularly."),
    wxTRANSLATE("Recommendation: If upgrading to a new version of MMEX, make sure you backup your .mmb database file before doing so."),
    wxTRANSLATE("Recommendation: Use copy (Ctrl+C) and paste (Ctrl+V) for frequently used transactions."),
    wxTRANSLATE("Tip: Remember to make backups of your .mmb database file."),
    wxTRANSLATE("Tip: The .mmb database file is not encrypted. That means anyone else having the proper know how can actually open the file and read the contents. So make sure that if you are storing any sensitive financial information it is properly guarded."),
    wxTRANSLATE("Tip: To mark a transaction as reconciled, select the transaction and press the “R” key. To mark a transaction as unreconciled, select the transaction and press the “U” key."),
    wxTRANSLATE("Tip: To mark a transaction as requiring followup, select the transaction and press the “F” key."),
    wxTRANSLATE("Tip: MMEX supports printing of all reports that can be viewed. The print options are available under the menu, File → Print."),
    wxTRANSLATE("Tip: You can modify some runtime behavior of MMEX by changing the settings in Options."),
    wxTRANSLATE("Tip: To print a statement with transactions from any arbitrary set of criteria, use the transaction filter to select the transactions you want and then do a print from the menu."),
    wxTRANSLATE("Tip: Set exchange rate for currencies in case if you have accounts with different currencies."),

    wxTRANSLATE("Category Manager tip: Pressing the “H” key will cycle through all categories starting with the letter h."),
    wxTRANSLATE("Category Manager tip: Pressing 2 key combination will cycle through all categories starting with that key combination. Example: Pressing ho will select Homeneeds, Home, House Tax, etc…"),

    wxTRANSLATE("Never endorse a loan. If the borrower—your friend, family member, significant other, whoever—misses payments, your credit score will take a plunge, the lender can come after you for the money, and it will likely destroy your relationship. Plus, if the bank is requiring a cosigner, the bank does not trust the person to make the payments."),

    wxTRANSLATE("Grocery tip: Avoid grocery shopping when hungry or stressed. Hunger and stress can make you do strange things, like buying lots of chocolate or gimmicky processed foods. Plan to go to the store when you are well-fed and relaxed. That way, you can stick to your list and not overspend on food you do not need."),
    wxTRANSLATE("Grocery tip: Make a shopping list. A shopping list saves money by helping you avoid buying products you do not need. The simplest way is to track consumables as you use them. Keep a magnetic notepad on the fridge and jot down what you need as you run out."),
    wxTRANSLATE("Grocery tip: Buy fruits and vegetables in season."),
    wxTRANSLATE("Grocery tip: Make sure your family consumes everything you purchase. Every month or so, hold a pantry challenge during which you use up as many pantry goods as possible before you shop again. A pantry challenge cuts down on waste and lets you get a sense of how much you are overbuying."),
    wxTRANSLATE("Grocery tip: Reduce waste. Adopting better grocery shopping habits is one way to cut back on wasted food and money. Make a list of everything you throw out without using, such as spoiled leftovers or soggy lettuce, to get a sense of the full scale of the issue."),
    wxTRANSLATE("Grocery tip: Read sales flyers. Review each store’s flyer and match what is on sale with what is on your list or plan meals around sale items.  It is sometimes more cost-effective to visit multiple stores in one week to get the best value."),

    wxTRANSLATE("Cut back on subscriptions. Do you really need to subscribe to every streaming service? Are you using a paid service enough to justify the expense, or can you survive with the ad-supported free one? Do you need that monthly book club, or can you head to the library once per month instead?"),

    wxTRANSLATE("Software tip: Replace closed-source software with free/libre open-source software. Visit https://alternativeto.net/"),
    wxTRANSLATE("Software tip: Become productive with “LibreOffice Base” for databases. Visit https://www.libreoffice.org/discover/base/"),
    wxTRANSLATE("Software tip: Become productive with “LibreOffice Calc” for spreadsheets. Visit https://www.libreoffice.org/discover/calc/"),
    wxTRANSLATE("Software tip: Become productive with “LibreOffice Draw” for vector graphics and flowcharts. Visit https://www.libreoffice.org/discover/draw/"),
    wxTRANSLATE("Software tip: Become productive with “LibreOffice Impress” for presentations. Visit https://www.libreoffice.org/discover/impress/"),
    wxTRANSLATE("Software tip: Become productive with “LibreOffice Math” for formula editing. Visit https://www.libreoffice.org/discover/math/"),
    wxTRANSLATE("Software tip: Become productive with “LibreOffice Writer” for word processing. Visit https://www.libreoffice.org/discover/writer/"),
    wxTRANSLATE("Software tip: Consolidate all your email, calendar, and contacts using the “Betterbird” client. Visit https://betterbird.eu/"),
    wxTRANSLATE("Software tip: Create images and edit photographs with “GNU Image Manipulation Program”. Visit https://www.gimp.org/"),
    wxTRANSLATE("Software tip: Enjoy “Linux Mint” a free and open-source operating system for your desktop device. Visit https://linuxmint.com/"),
    wxTRANSLATE("Software tip: If you know another language, help translate MMEX Money Manager at Crowdin. Visit https://crowdin.com/project/moneymanagerex"),
    wxTRANSLATE("Software tip: Merge, split, rotate, crop and rearrange PDF documents with “PDF Arranger”. Visit https://github.com/pdfarranger/pdfarranger"),
    wxTRANSLATE("Software tip: Share MMEX Money Manager with your friends. Visit https://moneymanagerex.org/"),
    wxTRANSLATE("Software tip: Share, freely and privately with “Jami” a free/libre, end-to-end encrypted, and private communication software. Visit https://jami.net/"),
    wxTRANSLATE("Software tip: Take handwritten notes with ease using “Xournal++”. Visit https://xournalpp.github.io/"),
    wxTRANSLATE("Software tip: Use “Qalculate!” the ultimate desktop calculator, powerful and versatile yet simple to use having useful tools for everyday needs (such as currency conversion and percent calculation). Visit https://qalculate.github.io/"),
    wxTRANSLATE("Software tip: Watch free and live TV, movies and series from around the world with “Hypnotix”. Visit https://github.com/linuxmint/hypnotix"),
    wxTRANSLATE("Software tip: Watch YouTube more conveniently with “FreeTube”. Visit https://freetubeapp.io/"),

    wxTRANSLATE("Parent tip: If you are asked to be a guarantor for a loan for your college student, first check to see if your child has maxed out federal loan, grant, and scholarship options."),

    wxTRANSLATE("Get out of debt tip: Pay yourself 10% first. Put this into an account that is hard to touch. Make sure it is a chore to get the money out (you have to drive to the bank), so you will only tap it consciously and for major expenses."),
    wxTRANSLATE("Get out of debt tip: Establish an emergency fund."),
    wxTRANSLATE("Get out of debt tip: Stop acquiring new debt."),
    wxTRANSLATE("Get out of debt tip: Create a realistic budget for your expenses."),
    wxTRANSLATE("Get out of debt tip: Spend less than you earn."),
    wxTRANSLATE("Get out of debt tip: Pay more than the minimum."),
    wxTRANSLATE("Get out of debt tip: If you have a mountain of debt, studies show paying off the smallest debts can give you the confidence to tackle the larger ones."),
    wxTRANSLATE("Before going to a shop and buy something: take the time making a list of what you really need. In the shop buy what is on your list.")
};

static const wxString STOCKTIPS[] = {
    wxTRANSLATE("Using MMEX it is possible to track stocks/mutual funds investments."),
    wxTRANSLATE("To create new stocks entry the Symbol, Number of shares and Purchase prise should be entered."),
    wxTRANSLATE("Sample of UK (HSBC HLDG) share: HSBA.L"),
    wxTRANSLATE("Before you start investing, have a fully-funded emergency fund."),
    
    wxTRANSLATE("Before you start investing, have a fully-funded emergency fund."),
    wxTRANSLATE("Before you start investing, know your cash flow (have a budget)."),
    wxTRANSLATE("Before you start investing, pay off consumer debt — especially credit cards."),
    wxTRANSLATE("Before you start investing, make net worth your primary tracking metric."),
    wxTRANSLATE("Before you start investing, clarify your goals and priorities."),
    wxTRANSLATE("Before you start investing, make sure you understand investing basics."),
    wxTRANSLATE("Common mistake of investors: only buying companies you know."),
    wxTRANSLATE("Common mistake of investors: buying companies you like."),
    wxTRANSLATE("Common mistake of investors: buying buzzworthy companies."),
    wxTRANSLATE("Common mistake of investors: forgetting to rebalance."),

};
