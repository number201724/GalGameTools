*envstart|
		[syscurrent name="game"]
		[scenestart storage=&tf.startscenario]
*envplay|
		[sceneplay]
		[exit storage="start.ks" target=*end]

; シーン鑑賞
*replay|
		[scenestart storage=&tf.replay_scenario target=&tf.replay_target]
		[sceneplay]

; 終了時演出
*endrecollection|
		[cancelskip]
		[cancelautomode]
		[bgm stop=1000]
		[begintrans]
		[allimage hide delete]
		[endtrans normal sync]
		[endrecollection]


