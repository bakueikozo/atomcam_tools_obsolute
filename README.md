# atomcam_tools

これはATOMCam/ATOMCam2/AtomSwingの機能を拡張するツールキットです。
標準的な機能に満足できないユーザーが、各自の責任において機能を拡張するためのツール・スクリプトをまとめたものです。
利用にあたって、当然のことながらカメラメーカへの問い合わせなどは厳に慎んでください。
それ以外、自由に利用できますが、仮に悪用し、他人へ迷惑をかけた場合の責任は、その設定を行った者が負うべきものであることを理解してください。
質問、動作不具合、機能のリクエストは　Issues を立ててください。

## 実現される機能
- WebUI (Port: 80)
  - ATOMCamのアプリから設定できない追加機能について設定します。

- CIFS(Samba4.0)サーバー(Port:137,138,139,445)
  - SD-Cardの保存されている映像のフォルダーをLAN内にguestアカウントで共有します。

- NASへの保存
  - CIFS(smb)プロトコルでNASへSD-Cardへ保存している映像と同じものを保存します。
  - SMB1.0はSecurity的に非推奨のため対応しません。2.0以上に対応のNASを使用してください。

- RTSPServer(Port:8554)
  - RTSP streaming を送出します。

- avahi(mDNS)機能(Port:5353)
  - microSDカードのhostnameファイルを編集することでデバイス名を変更できます（WebUIからも変更可能）
  - hostnameの命名規則は英数と-(hyphen)のみ（RFC952,1123で規定)です。\_(underscore)は使用できません。defaultはatomcamになっています。
  - mDNS対応しているOS（Windows10以降/MacOS/avahi入りlinux）からは[hostname].localでアクセスできるようになります。

- sshd (Port:22)
  - microSDカードのroot directoryにsshの公開鍵をauthorized\_keysの名前のファイルで置いてください。rootアカウントなのでパスワードではloginできない設定になっています。
  - ssh root@[ATOMCamのIPアドレス] or ssh root@[hsotname].local でloginできます。

- webHook機能(experimental)
  - 各種イベント発生時に指定したURLにpostで通知します。

- 動体検知アラームの不感知期間を短縮(experimental)

  - Atomcamの動体検知は一度検知すると５分間検知しない仕様ですが、この検知しない期間を30秒に短縮します。
  - メーカーへの迷惑防止のためCloudへの通知、video/jpegのuploadは５分以内の再送をブロックしています。このため、アプリへの通知も５分以内の再検知時は通知されません。
  - SD-Card, NASには記録されます。(検知時の12秒間のファイル、検知時を含む１分間のファイル共）
  - webHook機能もイベントごとに発生します。必要な場合はwebHook経由で通知を組んでください。RasberryPi上でNode-REDを動かすのがお手軽です。

- atomcam_toolsのupdate機能
  - GitHubのLatestイメージをダウンロードして更新する機能です。
  - 回線状況にもよりますが、３分程度かかります。
  - AtomCamのFWのupdateはできません。アプリからupdateをしてください。

- AtomCamのFW update対応
  - アプリからAtomCamのFW updateをするときにSD-Cardを抜かなくてもできるようになりました。
  - 商品とのubootと同じシーケンスでflashメモリ内の領域の消去と書き込みが行われます。
  - update途中で電源が落ちることがないよう気をつけてください。

- AtomSwingのpan/tilt制御(experimental)

  - WebUIからのpan/tilt操作

    但し自動追跡をonにしていると映像が動くことで物体認識して取り合いになります。

  - pan/tilt座標系の初期化のためのリセット動作をするボタンをメンテナンスに追加
## セキュリティに関わる重要事項
上記項目に書いてある各ポートが利用可能となります。  
現時点ではこのポートはセキュリティ上の懸念材料となりますので、  
ネットワークのセキュリティーを各自十分に保つように心がけてください。
WebUI、video、jpegに関してはLAN内からは自由に見える設定になっています。
sshは物理的にSD-Cardへアクセスして公開鍵を書かないとloginできないようにしています。
ただし、ATOMCamはSSID,passwordを平文でカメラ内のフラッシュメモリ（SD-Cardではない）に保存しているのでカメラを盗難されて中を見られるとWiFiにアクセスされる可能性がありますのでご注意ください。



## ファームウェアの更新の注意

 atomcam_toolsは以下のATOMCamのVersionで動作確認しています。更新した場合機能が使えなくなることもあるのでご注意ください。

ATOMCam Ver.4.33.3.64, 4.33.3.66

ATOMCam2 Ver.4.58.0.65, 4.58.0.71, 4.58.0.73, 4.58.0.79

ATOMSwing Ver.4.37.1.85, 4.37.1.90, 4.37.1.93



## 関連記事

Qiitaに少し解説を書いています。

[Qiita.com ATOMCam2を少し改造して導入してみた](https://qiita.com/mnakada/items/7d0fbcb6e629e1ddbd0c)

[Qiita.com AtomSwingを少し改造して遊んでみた](https://qiita.com/mnakada/items/5da19a302b0f7521f225)

## 使用法

https://github.com/mnakada/atomcam_tools/releases/latest

からatomcam_tools.zipをダウンロードし、適当なツールで解凍します。  
<img src="https://github.com/mnakada/atomcam_tools/blob/main/images/extract.png">

解凍されて出てきたすべてのファイルを、ATOMCamで使用可能なmicroSDカードのルートフォルダに保存します  
保存したmicroSDカードをATOMCamに入れて電源を入れます  


## Web設定画面

 http://atomcam.local を開くと設定画面にアクセスできます。

<img src="https://github.com/mnakada/atomcam_tools/blob/main/images/local_web.png">

mDNS未対応で開けない場合は、ATOMCam純正アプリや、IPアドレス確認ツールなどでATOMCamのIPアドレスを確認し、 ブラウザで http://[ATOMCamのIPアドレス] を開きます。

この設定画面で行った設定は microSDカード内、hack.ini　に保存され、次回再起動後からは自動的に読み込まれます。  


### 基本設定

#### デバイス名

カメラのデバイス名を設定します。
ここで設定した名前はCIFS(Samba) / mDNS(avahi) / NASのフォルダー名に使用されます。


### 録画

#### 検出通知のローカル録画

ATOMCamアプリで設定した検出時にクラウドサーバーに保存される12秒の映像をSD-Card/NASにも記録します。記録されるフォルダーは alarm_record です。

！！！　「録画ファイルの自動削除」で保存日数を指定しないと一杯になっても削除されません　！！！

「SD-Card消去」を押すことによってもファイル削除が行えます。  


#### ローカル録画スケジュール

スケジュールを選ぶと、曜日と時間帯を指定する項目が追加されます。

ATOMCamアプリの「録画およびストレージ管理」の「ローカル録画」で記録される映像の記録する時間帯を設定します。

右端のー/＋で指定項目を削除/追加できます。複数の項目はor条件で効きます。


### 記録メディア

#### SD-Card

offにするとATOMCamアプリの「Micro SDへのローカル録画」がonでもSD-Cardに記録されなくなります。

NASへのみ記録する場合はここをoffにしてください。（ただしフォルダーだけできてしまいます※そのうち対応します）

##### - ネットワークアクセス

onにするとSD-Cardの /record, /time_lapse, /alarm_record をSamba4.0でデバイス名のネットワークフォルダとしてLAN内に公開します。

​		**※ 負荷が重いためストリーミングRTSPと同時使用は推奨しません。**

##### 保存するPATH

SD-Cardのalarm_reocrdフォルダ以下のファイルのPATHを設定できます。書式はstrftimeの指定形式です。最後の/以降はファイル名として後ろに.mp4が付加されます。

recordフォルダ、time_lapseフォルダはアプリからアクセスされるためPATHの指定はできません。

例えば %Y%m%d/%H%M%S を指定すると/alarm_record/20211128/223000.mp4 というようなファイルが作られます。 

フォルダがない場合は自動で作成します。

##### ファイルの自動削除

SD-Cardに録画しているファイルを自動削除する機能です。

自動で削除したくない場合はoffにしてください。

容量にご注意ください。

##### 保存日数

録画ファイルを保存しておく日数です。この日数経過すると自動的に削除されます。

#### NAS

ATOMCamアプリの「Micro SDへのローカル録画」、この設定画面の「検出通知のローカル録画」の映像をNASにも記録します。

##### - ネットワークパス

NASのホスト名＋フォルダー名を//\[ホスト名]/[フォルダー名] の形式で指定します。

##### - アカウント

NASにアクセスするためのアカウント（ユーザー名）を指定します。

##### - パスワード

NASにアクセスするためのパスワードを指定します。（このパスワードは生値でSD-Cardに保存されます）

##### 保存するPATH

NASのalarm_record, recordフォルダ以下のファイルのPATHを設定できます。書式はstrftimeの指定形式です。最後の/以降はファイル名として後ろに.mp4が付加されます。

例えば %Y%m%d/%H%M%S を指定するとNASのrecordフォルダの場合、 //NASname/atomcam/record/20211128/223000.mp4 というようなファイルが作られます。 

フォルダがない場合は自動で作成します。

##### ファイルの自動削除

NASに録画しているファイルを自動削除する機能です。

自動で削除したくない場合はoffにしてください。

容量にご注意ください。

##### 保存日数

録画ファイルを保存しておく日数です。この日数経過すると自動的に削除されます。


### ストリーミング

#### RTSP (not recommended)

チェックを入れると、RTSPストリーミングを行います。 

特に、システムの安定性に関わるため、利用には十分なテストを行ってください。  

**※ 負荷が重いためSD-Cardのネットワークアクセスと同時使用は推奨しません。**

##### - 音声

RTSPの音声をon/offします。音声をonにすると負荷が大きくなるので、必要でなければoffにしておいてください。

##### - URL

VLC media playerの「ネットワークストリーミングを開く」で入力するURLが表示されます。


### イベント通知

#### WebHook(experimental)

動体検知や録画ファイルの書き込み等のイベントのタイミングで指定のURLに通知します。

##### - 通知URL

WebHookを受け取るURLを指定します。今のところ実験的な実装なのでLAN内のnon-secureなpostを想定しています。

{ type: 'event名', data: あれば何か }の形式でpostします。

##### - 動体検知

動体検知が働いた時に通知URLに type: alarmEvent をpostします。

##### - 動体認識情報

動体検知の認識情報取得時に通知URLに type: recognitionNotify, data: recognition data をpostします。

##### - 動体検知録画終了

動体検知での録画が終了した時に通知URLに type: uploadVideoFinish をpostします。

##### - 動体検知録画転送

動体検知での録画が終了した時に通知URLに mime:video/mp4で録画ファイル をpostします。

##### - 動体検知静止画保存

動体検知での静止画保存完了時に通知URLに type: uploadPictureFinishをpostします。

##### - 動体検知静止画転送

動体検知での静止画保存完了時に通知URLに mime:image/jpegで静止画ファイルをpostします。

##### - 定常録画保存

１分間の定常録画が終了するたびに通知URLに type: recordEventをpostします。

##### - タイムラプス記録

タイムラプス記録のたびに通知URLに type: timelapseEvent, data: timelapse pathをpostします。

##### - タイムラプス録画終了

タイムラプス録画終了時に通知URLに type: timelapseFinish, data: timelapse pathをpostします。

### 動体検知

#### 動体検知周期の短縮

動体検知アラームの不感知期間を５分から30秒に短縮します。

設定変更時には反映するために再起動されます。

### メンテナンス

#### Swing座標初期化

Swingのpan/tilt座標系を初期化します。

両側の端点に当てることでモーターの動作範囲をリセットさせるための動作をします。

#### 定期リスタート

カメラのシステムを指定したスケジュールで再起動します。

ネットワークの不調など、何らかの理由でATOMCamが連続稼働することができない場合の対応ですが、必ずしもこれによって問題が解決するとは限りません。  

#### リブート

AtomCamを再起動します。

Lockスイッチを解除してRebootボタンを押してください。

再起動に60~80秒くらいかかります。

#### SD-Card消去

SD-Cardのrecord, alarm_record, time_lapseフォルダの中身を消去します。

SD-Cardにtoolが入っているため、アプリからのSD-Cardのフォーマットをdisableしています。

その代替手段として用意しています。

Lockスイッチを解除してからEraseボタンを押してください。

#### Update

GitHubのLatest VersionにUpdate します。GitHubからLatest Versionをダウンロードして展開して書き込み再起動します。180秒くらいかかります。

 現在のtoolのバージョン（タイトル部に表示されています）がLatest Versionより古い場合のみUpdateすることができます。

台数が多い場合や回線が細い場合、PCで[GitHubのLatest Version](https://github.com/mnakada/atomcam_tools/releases/latest)のatomcam_tools.zipをダウンロードし、展開せずにそのままSamba経由でSD-Cardのupdateフォルダに入れて、リブートすることでもUpdateできます。この場合はVerのチェックは行われません。


### Copyright

LICENSEファイルを参照してください


### 寄付について
このアプリの使用条件は特にありませんが、以下のリンクから買い物をすると、売り上げの一部が大元のhackをしてくれたhoneylabさんに還元されます。
https://honeylab.hatenablog.jp/entry/2021/09/29/115855
