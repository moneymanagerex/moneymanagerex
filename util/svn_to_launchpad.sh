#!/bin/bash
EMAIL="moneymanagerex@moneymanagerex.com"
PPA_N=1
StableSvnList="http://svn.code.sf.net/p/moneymanagerex/code/tags/releases/"

spinner()
{
p=$1
while [ -d /proc/$p ]
do
echo -en '\E[32;40m/' ; sleep .05
echo -en '\E[32;40m-' ; sleep .05
echo -en '\E[32;40m\' ; sleep .05
echo -en '\E[32;40m|' ; sleep .05
echo -en '\E[32;42mS' ; tput sgr0 ; sleep .2
done
}
vercomp () {
    if [[  $1 == $2 ]]
    then
        echo "Version of MMEX the same, overwrite PPA? Y(n)"
		read -n 1 Replace
		case $Replace in
			"дД" | "yY" | "") PPA_N=$PpaVersion + 1;;
			"нН" | "nN") exit 1;;
			"*") echo "Imcorrect choice! Breaking..."; exit 1;;
		esac
    fi
    local IFS=.
    local i ver1=($1) ver2=($2)
    # fill empty fields in ver1 with zeros
    for ((i=${#ver1[@]}; i<${#ver2[@]}; i++))
    do
        ver1[i]=0
    done
    for ((i=0; i<${#ver1[@]}; i++))
    do
        if  [[ -z ${ver2[i]} ]]
        then
            # fill empty fields in ver2 with zeros
            ver2[i]=0
        fi
        if ((10#${ver1[i]} > 10#${ver2[i]}))
        then
        	echo "Обнаружена новая стабильная версия пакета $StableSvnVersion. Обновляем. Нажмите Enter для продолжения"
			read temp 
        fi
        if ((10#${ver1[i]} < 10#${ver2[i]}))
        then
            echo "Ошибка, версия пакета в SVN меньше, версии уже загруженного на Launchpad пакета!"
        fi
    done
return 0
}

#Getting last stable release number
echo "Получение данных об актуальной стабильной версии SVN..."
StableSvnVersion=`svn list $StableSvnList | tail -1 | sed 's/\///g'` #&  #detect newest stable release
#spinner $!
echo "Текущая актуальная версия в SVN: $StableSvnVersion"
StableSvnRepo="$StableSvnList/$StableSvnVersion"
UnstableSvnRepo="http://svn.code.sf.net/p/moneymanagerex/code/trunk/"
GpgKey=`gpg --list-keys | grep ^pub | awk '{print $2}' | cut -d '/' -f2`


echo "Обновления списка пакетов..."
apt-get update &>/dev/null &
spinner $!

	
case $1 in
	"stable")
	LaunchpadVersion=`apt-cache policy mmex | awk '/-stable/{print s;} {s=$0}' | awk '{for(i=1;i<=NF;i++){if($i~/ppa/){print $i}}}' | cut -d "~" -f1` #get lauchpad stable version before "~"
	PpaVersion=`apt-cache policy mmex | awk '/-stable/{print s;} {s=$0}' | awk '{for(i=1;i<=NF;i++){if($i~/ppa/){print $i}}}' | cut -d "~" -f2 | sed 's/ppa//g'` #get lauchpad stable version behind "~ppa"
	SvnRepo=$StableSvnRepo
	DownloadFolder="mmex"
	PpaType="ppa-stable"
;;

	"unstable")
	LaunchpadVersion=`apt-cache policy mmex | awk '/-unstable/{print s;} {s=$0}' | awk '{for(i=1;i<=NF;i++){if($i~/ppa/){print $i}}}' | cut -d "+" -f1` #get launchpad unstable version
	LaunchpadRevVersion=`apt-cache policy mmex | awk '/-unstable/{print s;} {s=$0}' | awk '{for(i=1;i<=NF;i++){if($i~/ppa/){print $i}}}' | cut -d "+" -f2 | cut -d "~" -f1` #get launchpad rev version
	PpaVersion=`apt-cache policy mmex | awk '/-unstable/{print s;} {s=$0}' | awk '{for(i=1;i<=NF;i++){if($i~/ppa/){print $i}}}' | cut -d "+" -f2 | cut -d "~" -f2` #get launchpad ppa version
	SvnRepo=$UnstableSvnRepo
	DownloadFolder="mmex-unstable" 
	PpaType="ppa-unstable"
;;
	*) echo -e "\nОшибка! Порядок использования: ${0##*/} <тип PPA (stable или unstable)>"; exit 65;;

esac
echo "Текущая актуальная версия Launchpad пакета: $LaunchpadVersion"
echo "Текущая актуальная ревизия Launchpad пакета: $LaunchpadRevVersion"
echo "Получение исходных кодов из SVN"
if [ ! -d $DownloadFolder ]; then
	SvnRevVersion=`svn co $SvnRepo $DownloadFolder | grep "редакц" | awk '{print $NF}' | sed 's/\.//g'`
	else
	cd $DownloadFolder
	SvnRevVersion=`svn up | grep "редакц" | awk '{print $NF}' | sed 's/\.//g'`
	cd ..
fi
echo "Текущая актуальная ревизия SVN исходников: $SvnRevVersion"
case $1 in
	"stable")
		vercomp $LaunchpadVersion $StableSvnVersion
		SourceFolder=$DownloadFolder-$StableSvnVersion~ppa$PPA_N
		;;
	"unstable")
		   if [[ $LaunchpadRevVersion == $SvnRevVersion ]]; then
        		echo "Ревизии MMEX совпадают, заменить текущую ревизию в PPA? Н(д)"
				read Replace
				case $Replace in
					[дД] | [yY]) PPA_N=$[PPA_N + 1]; echo $PPA_N;;
					[нН] | [nN] | "") echo "Выход..."; exit 0;;
					*) echo "Ошибка выбора! Прерываемся..."; exit 0;;
				esac
			else
				if [[ $LaunchpadRevVersion < $SvnRevVersion ]]; then
					echo "Получена версия $StableSvnVersion+$SvnRevVersion из SVN обновляем PPA"
				else
					echo "Ошибка! Номер ревизии в SVN меньше, чем номер ревизии уже загруженного на Launchpad пакета!"; exit 0
				fi
			fi
			SourceFolder=$DownloadFolder-$StableSvnVersion+$SvnRevVersion~ppa$PPA_N
	;;
esac
cd $DownloadFolder
#if [ $DownladFolder == "mmex" ]; then
#	SourceFolder=$DownloadFolder-$StableSvnVersion~ppa$PPA_N
#	else
#	SourceFolder=$DownloadFolsder-$StableSvnVersion+$SvnRevVersion~ppa$PPA_N
#fi
rsync -rCh . ../$SourceFolder
cd ..
SourceFolderUnderline=`echo "$SourceFolder"|sed s/-$1-/_/g`
tar czf $SourceFolderUnderline.orig.tar.gz $SourceFolder
cd $SourceFolder
PatchAmount=`ls setup/linux/debian|grep diff|wc -l`
if [ -n $PatchAmount ]; then
	while :
	do
		echo -e "Найдено $PatchAmount патч(ей) для исходников. \nПрименить? Д(н) (v - просмотр, правка патчей)"
		read PatchQestion
		case $PatchQestion in
			[Дд]|[yY]|"")
				cp setup/linux/debian/*.diff .
				patch -p0 < *.diff
				break
			;;		
			[Нн]|[Nn])
				break	
			;;
			[Vv])
			echo -e "Файл патча откроется в окне редактора nano. Если патчей несколько они будут открываться по очереди после закрытия предыдущего. Для сохранения изменний в nano нажмите Ctrl+O, для выхода нажмите Ctrl+X.\n\nДля продолжения нажмите Enter"
			read temp
			nano setup/linux/debian/*.diff
			;;
			*)
				echo -e "\nОтвет не подходит."
			;;
		esac
	done
fi
./bootstrap
dh_make -s -e $EMAIL -c gpl3 -n
cp ../$DownloadFolder/setup/linux/debian/control debian/
if [ $1 == "stable" ];then
	sed "1 s/.*/mmex ($StableSvnVersion~ppa$PPA_N) precise; urgency=low/" debian/changelog > debian/changelog.temp
	mv debian/changelog.temp debian/changelog
	else
	sed -i "1 s/.*/mmex ($StableSvnVersion+$SvnRevVersion~ppa$PPA_N) precise; urgency=low/" debian/changelog > debian/changelog.tmp
	mv debian/changelog.temp debian/changelog
fi
debuild -S -sd -k$GpgKey
cd ..
dput $PpaType $SourceFolderUnderline\_source.changes 
