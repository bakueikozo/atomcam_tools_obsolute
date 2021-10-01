# atomcam_tools

これはATOMCam2の機能を拡張するツールキットです  
このツールはATOMCam2の標準的な機能に満足できないユーザーが、  
各自の責任において機能を拡張するためのツール・スクリプトをまとめたものです。  
利用にあたって、当然のことながらカメラメーカへの問い合わせなどは厳に慎んでください。  
それ以外、自由に利用できますが、仮に悪用し、他人へ迷惑をかけた場合の責任は  
その設定を行った者が負うべきものであることを理解してください。  
質問、動作不具合、機能のリクエストは　Issues を立ててください。  
どうしてもわからない場合は、twitterで関連する投稿にリプライを付けてください。  
telnetパスワード以外のことについてのDMでの質問は、他の方への情報共有が滞りますのでもったいないです。

## 実現される機能
- WebUI (Port: 8080)
- FTPサーバー機能 (Port:21)
- Telnet (Port:23)
  - rootユーザー、パスワードはこのこのカメラ(2世代目)の商品名を英数小文字、8文字で入力したものです。  
      - わからなければTwitterのDMで聞いてください。  
      - microSDカードのroot directoryにdisable\_telnetという名前のファイルをつくるとtelnetをdisableすることができます。
- avahi(mDNS)機能
  - microSDカードのroot directoryにAtomCamの名前をhostnameのファイル名で書いてください。
  - hostnameの命名規則は英数と-(hyphen)のみ（RFC952,1123で規定)です。\_(underscore)は使用できません。defaultはatomcamになっています。
  - mDNS対応しているOSからは[hostname].localでアクセスできるようになります。
- sshd (Port:22)
  - microSDカードのroot directoryにsshの公開鍵をauthorized\_keysの名前のファイルで置いてください。rootアカウントなのでパスワードではloginできない設定になっています。
  - ssh root@[ATOMCam2のIPアドレス] or ssh root@[hsotname].local でloginできます。
## セキュリティに関わる重要事項
上記項目に書いてある各ポートが利用可能となります。  
現時点ではこのポートはセキュリティ上の懸念材料となりますので、  
ネットワークのセキュリティーを各自十分に保つように心がけてください。

## 使用法

https://github.com/bakueikozo/atomcam_tools/releases/tag/v1.0rc

からatomcam_tools.zipをダウンロードし、適当なツールで解凍します。  
<img src="https://github.com/bakueikozo/atomcam_tools/blob/main/images/extract.png">

解凍されて出てきたすべてのファイルを、ATOMCam2で使用可能なmicroSDカードのルートフォルダに保存します  
保存したmicroSDカードをATOMCam2に入れて電源を入れます  

## Web設定画面

ATOMCam純正アプリや、IPアドレス確認ツールなどでATOMCam2のIPアドレスを確認し、  
ブラウザで http://[ATOMCam2のIPアドレス]:8080/cgi-bin/honeylab.cgi を開きます  
この設定画面で行った設定は microSDカード内、hack.ini　に保存され、次回再起動後からは自動的に読み込まれます。  

### Alarm File Save
Save Alarm MP4 into alarm_record folder (Note: No remove automatically.)  
にチェックを入れると、検出機能によって生成された12秒動画を内蔵microSDカードのalarm_recordフォルダに保存します。  
！！！　このファイルは自動的には削除されないため、FTPサーバ機能などを使って随時削除する必要があります。！！！  
または、「Clean Alarm Save Folder」を押すことによってもファイル削除が行えます。  

### FTP Setting
「Enable FTP Server」にチェックを入れると、FTPサーバ機能が起動します(再起動が必要です)  
「Working as FTP Client」にチェックを入れると、FTPクライアントとして動作し、  
特定のFTPサーバに毎分録画を送信することができます  
#### FTP Server IP Address
 FTPサーバのアドレスを入力します
#### Login User as
 FTPサーバに登録されたユーザ名を入力します
#### Password
 FTPサーバに登録されたパスワードを入力します
#### upload folder
 各カメラごと違う名前で、動画をアップロードするフォルダ名を指定します。

#### ALL NORMAL RECORD FILES
 毎分録画を [upload folder]/record/yyyy/mm/dd/yyyymmdd_hhmm.mp4 として転送します
#### ALARM RECORD FILES
 検出動画(12秒)を [upload folder]/alarm_record/yyyymmdd_hhmm.mp4 として転送します。

### Schedule Recording
録画された毎分動画を残す設定を行います。
#### ALL HOURS
これにチェックを入れておくと、すべての録画を行う通常の動作となります。
#### CUSTOM
これにチェックを入れておくと、曜日・時刻マトリックスのチェックボックスがONになっていない時間は、 
録画されたファイルを随時削除します。  
明らかに不要な時間帯を除外することで、長い日数の録画を残せるようにするための試みです  

### Swap file on SD Card
#### Swap file size MB ( 0 = disabled)  (Need to reboot)
動作を安定させるための仮想メモリのサイズを設定します。
RTSPサーバを使用するためには、40MB程度のサイズを設定しておくと安定性が高まることが確認されています。
また、この場合microSDへの書き込み回数が多くなるため、信頼性の高いディスクを使用することが必要です。

### Reboot Setting
#### Reboot in each  x Hours.( 0 = disabled) 
ネットワークの不調など、何らかの理由でATOMCamが連続稼働することができない場合、  
一定時間ごとに再起動を試みます。  
必ずしもこれによって問題が解決するとは限りません。  

### RTSP setting (not recommended)
#### USE RTSP Streaming (Need to reboot)
チェックを入れると、RTSPストリーミングを行います。 　
特に、システムの安定性に関わるため、利用には十分なテストを行ってください。  
#### streaming URL 
VLC media playerの「ネットワークストリーミングを開く」で入力するURLが表示されます。


### Copyright
LICENSEファイルを参照してください

### 寄付について
このアプリの使用条件は特にありませんが、以下のリンクから買い物をすると、売り上げの一部が私に還元されます。
https://honeylab.hatenablog.jp/entry/2021/09/29/115855
