Translation
===========

MMEX uses .po files to store language strings for the program. The default
.po file is in English. A translation of the .po file will be done by simply
writing the translated strings in your language and then saving the file
with a unique name. This file will be loaded when the user selects your
language.

As MMEX is developed and more features are added, new strings might be added
to the default.po file. This might cause the translated .po files to be out
of date. But do not worry about this because for every translation it cannot
find in the language specific .po file the program will automatically fall
back to the English string. This will continue to be the case till the .po
file is updated.

Everyone likes to be acknowledged for their efforts. So once you become
a translator for a particular language, you will be listed in the credits
of the program in the "About" dialog. Every once in a while, you might need
to update the translated .po file with new strings that may have been added
in MMEX since then.

Procedure
---------

+ Installing PoEdit: Download [poEdit]  (if running Windows, just get the
  simple installer version) and install it.

+ Getting the proper [.po file]: Change a branch by pulldown menu for
  a proper branch.

+ Opening the file: Open poEdit (Start->Programs->poEdit->poEdit). If this
  is the first time you've run it, it will ask for your name and email
  address, to put them in the translation file. Then go to File->Open and
  open up the english-mmex file. Don't be worried by the number of strings
  to translate! You do not have to translate every line before sending us
  what you have done - MMEX will simply default to English for anything yet
  to be translated.

+ Basic translation: Select a line to translate in the top frame, and enter
  the translation in the bottom. poEdit keeps track of which lines have not
  yet been translated, and which are "fuzzy" translations (automated
  translations that may be wrong and should be checked). poEdit will also
  tell you if a sentence has a plural (ie. needs a different translation
  depending on the number of things being printed out). This should be
  straightforward. File->Save or the Save button will save your translations
  to the same file.

+ % placeholders: The % placeholders are programming constructs used to
  denote something will be filled in at runtime. Simply leave them as is in
  the translated string. For example, a sentence to give the colour of a cat
  and its size, in Spanish:
  ```
  "The cat is %s and %d centimetres long."
  "El gato tiene %s y %d centímetros de largo"
  ```

+ & and formatting: Please preserve any '&' characters or spaces
  in the strings as always.

+ Testing your translation: In your MMEX (applies to 0.5.1.0 and above)
  installation, simply delete the english-mmex.po/mo files, rename your
  translated files to english-mmex.po/mo files and make sure you have
  selected the language as English. Now your translated strings should appear!

+ Post your translated .po file on the [forum] or Email it to zealsupport
  mailing list (you are on there aren't you) with as much or as little as
  you have done, and I will get it up and running as soon as possible. :-)

+ So your .po language file has been added to the MMEX install. Good.
  MMEX is now localized for your language. However, as new versions come
  out, new strings are added to MMEX and you'll need to update your .po file
  accordingly. Every new release, the .po files in the install will be
  updated with new strings. You need to download the latest [release] and
  open the .po file and update it with new translations for the strings.
  Then upload the file into the [forum].

[poEdit]: http://poedit.net
[.po file]: https://github.com/moneymanagerex/moneymanagerex/tree/master/po
[release]: https://github.com/moneymanagerex/moneymanagerex/releases
[forum]: http://forum.moneymanagerex.org/viewforum.php?f=6