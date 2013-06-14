If you add new .po file, you should add new rule to the po.bkl.

For example, add this line for "some_new_language.po":
<msgfmt id="some_new_language.mo"/>

Also add new language to InnoSetup script at trunk\mmex\setup\win32\mmex.iss,
section [Components]. For example:
Name: "lang\some_new_language"; Description: "some_new_language"; Types: full
