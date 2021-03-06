*
	; 選択肢配置領域の指定
	;[selopt left=0 top=60 width=800 height=360 shadow bold shadowColor=0 color=0xCBCACB overColor=0xFFFFFF]
	;normal="select_normal" over="select_over" entersebuf=8 clicksebuf=9
	;enterse='' clickse=''
	[selopt msgoff uistorage=select left=0 top=110 width=1280 height=400 fadetime=300 size=22 edge edgeColor=0 edgeExtent=2 edgeEmphasis=2048]

	; ヒストリレイヤの uipsd
	[historyopt storage=backlog]

	; ゲーム中の右クリックメニューのデフォルト設定を変更
	; [sysrclickopt enabled=true call=true storage=sysmenu.ks target=""]

	; メッセージウィンドウの uipsd
	; メッセージウィンドウのオプション
	[meswinopt layer=message0 storage=message transparent=true visible=false nameAlign=-1 nameVAlign=0 opacity=&0.7*255 faceabsolute=1 nameabsolute=2 textabsolute=3 faceorigin=5]

	; 辞書
	;;[encyclopedia color=0xFFC0C0]

	[addSysScript name="game" storage="start"]
;;	[addSysScript name="schelp" storage="custom.ks" target="*ShortcutHelp"]

	[addSysHook   name="first.logo"    call storage="custom.ks" target=*logo]
	[addSysHook   name="exit.begin"    call storage="custom.ks" target=*exit]

	[addSysHook   name="title.loop"  jump storage="custom.ks" target=*title]
	[addSysHook   name="title.extra" jump storage="custom.ks" target=*extra]
	[addSysHook   name="title.game"  call storage="custom.ks" target=*title_game]

	[addSysScript name="title.from.option"       storage="custom.ks" target=*title_restore_option]
	[addSysScript name="title.from.load"         storage="custom.ks" target=*title_restore_load]

	[addSysScript name="title.from.soundmode"    storage="custom.ks" target=*title_restore]
	[addSysScript name="title.from.cgmode"       storage="custom.ks" target=*title_restore]
	[addSysScript name="title.from.scenemode"    storage="custom.ks" target=*title_restore]

	[addSysHook   name="scenemode.view.init"     call storage="custom.ks" target=*recollection_start]
	[addSysHook   name="scenemode.start.restore" call storage="custom.ks" target=*recollection_end]

	[addSysHook   name="loadinit.start"  call storage="custom.ks" target=*loadinit]
	[addSysHook   name="previnit.start"  call storage="custom.ks" target=*loadinit]

	[addSysHook   name="load.open.init"     call storage="custom.ks" target=*syswin_open]
	[addSysHook   name="save.open.init"     call storage="custom.ks" target=*syswin_open]
	[addSysHook   name="option.open.init"   call storage="custom.ks" target=*syswin_open]
	[addSysHook   name="backlog.open.init"  call storage="custom.ks" target=*syswin_open]
	[addSysHook   name="load.close.init"    call storage="custom.ks" target=*syswin_close]
	[addSysHook   name="save.close.init"    call storage="custom.ks" target=*syswin_close]
	[addSysHook   name="option.close.init"  call storage="custom.ks" target=*syswin_close_option]
	[addSysHook   name="backlog.close.init" call storage="custom.ks" target=*syswin_close]

	[syscover visible color=0xFFFFFF]
	[return]

*exit
	[quickmenu init]
	[return]

*logo
	[quickmenu init]

	;ロゴ表示
	[stoptrans]
	[clearlayers]
	[backlay]
	[event file=brandlogo notrans]
	[syscover visible color=0xFFFFFF]
	[sysupdate]

	[clickskip enabled=true]

	[syscover time=500 visible=false]
	[cancelskip]
	[beginskip]
	[rndvoice tag=brand]
	[wait time=2000]
	[endskip]
	[syscover visible=false]

	[beginskip]
	[begintrans]
	[event hide]
	[endtrans trans=crossfade time=500]
	[wait time=500]
	[endskip]

	[sysupdate]
	[set name="tf.caution" value=1]
	[return]

*caution
	[cancelskip]
	[clickskip enabled=true]
	[beginskip][ev file=caution1 shortfade][endskip]
	[wait time=2000]

;	[beginskip][ev file=caution2 shortfade][endskip]
	[return]

*title_bgm
	@bgm play=bgm_01
	@return

*title
	[call target=*caution cond=tf.caution]
	[touch file1="title_effect1_screen" file2="title_effect2_hardlight" file3="title_bg"]
	[touch file1="title_エルシア" file2="title_由宇" file3="title_明莉" file4="title_真奈"]
	[wait time=2000 cond=tf.caution]
	[cancelskip]
	[call target=*title_common]
	[beginskip]
	[wbl name=t1 canskip]
	[begintrans]
	[allimage hide delete]
	[clearlayers page=back]
	[ev file=title_bg]
	[title_effect1 show]
	[title_effect2 show]
	[endtrans fade=250]
	[endskip]
	[set name="tf.caution" value=0]
	[stopaction]
	[cancelskip]
	[beginskip]
	[wbl name=t2 canskip][title_由宇     show nosync]
	[wbl name=t3 canskip][title_明莉     show nosync]
	[wbl name=t4 canskip][title_真奈     show nosync]
	[wbl name=t5 canskip][title_エルシア show   sync]
	[begintrans]
	[allimage hide delete]
	[ev file=bg_c_white]
	[endtrans superquick2]
	[ev file=title_comp1 normal time=200 sync]
	[title_logo show nosync]
	[ev file=title_comp2 normal time=200 sync]
	[ev file=title_comp3 normal time=200 sync]
	[begintrans]
	[syspage uiload page=back]
	[endtrans quickfade]
	[endskip]
	[cancelskip]
	[rndvoice tag=title]
	[jump target=*title_wait]

*title_restore_option
	[syshook name="option.close"]
	[jump target=*title_restore]
*title_restore_load
	[syshook name="load.close"]
	[jump target=*title_restore]
*title_restore
	[call target=*title_common]
	[begintrans]
	[clearlayers page=back]
	[ev file=title_comp3]
	[title_logo show]
	[syspage uiload page=back]
	[endtrans fade=500]
	[jump target=*title_wait]

*title_common
	[call target=*title_bgm]
	[stoptrans]
	[rclick enabled=false]
	[clickskip enabled=true]
	[dialog name=title]
	[return]

*title_wait
	[syspage current page=fore]
	[eval exp='ExtraOnNotifier.CheckAndShow()']
	[jump storage=title.ks target=*wait]

*title_game
	[clickskip enabled=false]
	[bgm stop=1000]
	[begintrans]
	[allimage hide delete]
	[clearlayers page=back]
	[endtrans normal]
	[return]

*extra
	[jump storage="title.ks" target=*cgmode]

*loadinit
	[quickmenu fadeout time=&tf.fadetime]
	[return]

*syswin_open
	[position page=back layer=message1 left=0 top=0 frame=mask_syswin opacity=255 transparent=false visible=true]
	[return]
*syswin_close_option
	[syshook name="message.redraw"]
*syswin_close
	[syspage free page=back layer=message1]
	[return]


*saveload_edit
	[panel class="SaveDataEdit"]
	[dialog action="onSaveDataEdit"]
	[s]

*recollection_start
	[eval exp="tf.extraplaying=kag.bgm.playingStorage"]
	[allimage notrans hide delete sync]
	[return]

*recollection_end
	[allse stop]
	[bgm stop=1000]
	[begintrans]
	[allimage delete]
	[endtrans normal sync]
	[quickmenu init]
	[bgm play=&tf.extraplaying cond="tf.extraplaying!=''"]
	[call target=*title_bgm    cond="tf.extraplaying==''"]
	[return]

