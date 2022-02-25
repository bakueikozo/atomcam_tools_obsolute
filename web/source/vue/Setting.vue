<template>
  <div>
    <div class="title">
      <div>
        ATOMCam Hack
        <span class="version">
          Ver.{{ config.ATOMHACKVER }}
        </span>
      </div>
      <span class="atomcam">
        {{ config.PRODUCT_MODEL }}
        Ver.{{ config.appver }}
      </span>
      <span class="timestamp">
        {{ intervalValue.TIMESTAMP }}
      </span>
      <a href="https://github.com/mnakada/atomcam_tools#atomcam_tools" target="_blank" rel="noopener">
        <i class="el-icon-info help" />
      </a>
    </div>
    <div class="well-transparent container">
      <div class="image-frame" :style="imageFrameStyle">
        <div class="image-frame-inner1">
          <ElSlider v-if="isSwing" class="tilt-slider" v-model="tilt" :min="0" :max="180" vertical :show-input-controls="false" height="100%" @input="Move" />
          <ElTooltip :tabindex="-1" placement="top" :content="stillFullView?'clickで縮小します':'clickで拡大します'" effect="light" :open-delay="500">
            <img class="still-image" :src="`/cgi-bin/get_jpeg.cgi?r=${stillCount}`" @click="stillFullView=!stillFullView">
          </ElTooltip>
        </div>
        <div v-if="isSwing" class="image-frame-inner2">
          <ElSlider class="pan-slider" v-model="pan" :min="0" :max="355" :show-input-controls="false" @input="Move" />
        </div>
      </div>
      <h3>基本設定</h3>
      <ElTooltip :tabindex="-1" placement="top" content="NASの保存フォルダ名やリモートアクセスのための名前を設定します" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="1" :span="8">
            <h4>デバイス名</h4>
        </ElCol>
        <ElCol :span="4">
          <ElInput type="text" v-model="config.HOSTNAME" />
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="このページへのアクセスのためのログイン認証を有効にします" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="1" :span="8">
            <h4>ログイン認証</h4>
          </ElCol>
          <ElCol :span="3">
            <ElSwitch v-model="loginAuth" active-value="on" inactive-value="off" />
          </ElCol>
        </ElRow>
      </ElTooltip>
      <ElTooltip v-if="loginAuth==='on'" :tabindex="-1" placement="top" content="ログインのためのユーザーアカウントを設定します" effect="light" :open-delay="500">
        <ElRow>
          <ElCol :offset="1" :span="8">
            <h4>アカウント</h4>
          </ElCol>
          <ElCol :span="4">
            <ElInput type="text" v-model="account" />
          </ElCol>
        </ElRow>
      </ElTooltip>
      <ElTooltip v-if="loginAuth==='on'" :tabindex="-1" placement="top" content="ログインのためのパスワードを設定します" effect="light" :open-delay="500">
        <ElRow>
          <ElCol :offset="1" :span="8">
            <h4>パスワード</h4>
          </ElCol>
          <ElCol :span="4">
            <ElInput type="password" v-model="password" />
          </ElCol>
        </ElRow>
      </ElTooltip>

      <h3>録画</h3>
          <ElTooltip :tabindex="-1" placement="top" content="モーション／サウンド検出したときの12秒の映像をローカルにも録画します" effect="light" :open-delay="500">
        <ElRow>
          <ElCol :offset="1" :span="8">
            <h4>検出通知のローカル録画</h4>
        </ElCol>
        <ElCol :span="3">
          <ElSwitch v-model="config.RECORDING_ALARM" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="SD-Card/NASに録画される時間帯を設定します。" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="1" :span="8">
            <h4>ローカル録画スケジュール</h4>
        </ElCol>
        <ElCol :span="10">
            <ElSwitch v-model="config.RECORDING_LOCAL_SCHEDULE" active-value="on" inactive-value="off" active-color="#13ce66" inactive-color="#409eff" active-text="スケジュール " inactive-text="常時録画 " @change="(config.RECORDING_LOCAL_SCHEDULE === 'on') && !schedule.length && AddSchedule()" />
        </ElCol>
      </ElRow>
      </ElTooltip>

      <div v-if="config.RECORDING_LOCAL_SCHEDULE === 'on'">
        <ElRow v-for="(timeTable, idx) of schedule" :key="'timetable'+idx">
          <ElCol :offset="9" :span="12">
            <div class="well schedule">
              <div class="schedule-week">
                <ElCheckboxGroup v-model="timeTable.dayOfWeekSelect" size="small" :min="1">
                  <ElCheckboxButton v-for="day of weekDays" :label="day" :key="day">
                    {{ day }}
                  </ElCheckboxButton>
                </ElCheckboxGroup>
                <ElButton class="schedule-button" type="text" size="small" circle icon="el-icon-minus" @click="DeleteSchedule(idx)" />
              </div>
              <div class="schedule-week">
                <div class="schedule-time">
                  <ElSwitch v-model="timeTable.allDay" active-text="終日" />
                  <div class="schedule-timerange" v-if="!timeTable.allDay">
                    <ElTimePicker class="time-picker" v-model="timeTable.startTime" placeholder="開始時間" value-format="HH:mm" format="HH:mm" />
                    -
                    <ElTimePicker class="time-picker" v-model="timeTable.endTime" placeholder="終了時間" value-format="HH:mm" format="HH:mm" />
                  </div>
                </div>
                <ElButton class="schedule-button" type="text" size="small" circle icon="el-icon-plus" @click="AddSchedule" />
              </div>
            </div>
          </ElCol>
        </ElRow>
      </div>

      <h3>記録メディア</h3>
      <ElTooltip :tabindex="-1" placement="top" content="SD-Cardへの記録をします" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="1" :span="8">
            <h4>SD-Card</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.STORAGE_SDCARD" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip :tabindex="-1" placement="top" content="ATOMCamのSD-CardをCIFS(smb) serverとしてLAN内に公開します" effect="light" :open-delay="500">
      <ElRow v-if="config.STORAGE_SDCARD === 'on'">
        <ElCol :offset="2" :span="7">
            <h4>ネットワークアクセス</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.STORAGE_SDCARD_PUBLISH" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip :tabindex="-1" placement="top" content="録画するPATHをstrftimeの書式指定で記述します。最後に拡張子が付加されます。alarm_recordのみに有効。" effect="light" :open-delay="500">
      <ElRow v-if="config.STORAGE_SDCARD==='on'">
        <ElCol :offset="2" :span="7">
            <h4>保存するPATH</h4>
        </ElCol>
        <ElCol :span="6">
          <ElInput type="text" v-model="config.STORAGE_SDCARD_PATH" @input="FixPath('STORAGE_SDCARD_PATH')" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip :tabindex="-1" placement="top" content="SD-Cardに録画したファイルを自動的に削除します" effect="light" :open-delay="500">
      <ElRow v-if="config.STORAGE_SDCARD==='on'">
        <ElCol :offset="2" :span="7">
            <h4>ファイルの自動削除</h4>
        </ElCol>
        <ElCol :span="3">
          <ElSwitch v-model="config.STORAGE_SDCARD_REMOVE" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip v-if="config.STORAGE_SDCARD==='on' && config.STORAGE_SDCARD_REMOVE === 'on'" :tabindex="-1" placement="top" content="指定日数後に削除します" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>保存日数</h4>
        </ElCol>
        <ElCol :span="3">
          <ElInputNumber v-model="config.STORAGE_SDCARD_REMOVE_DAYS" :min="1" :step-strictly="true" size="mini" />
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="NAS(CIFS Server)への記録をします" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="1" :span="8">
            <h4>NAS</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.STORAGE_CIFS" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip v-if="config.STORAGE_CIFS === 'on'" :tabindex="-1" placement="top" content="NASのパスを設定。(//server/folder/の形式で指定)" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>ネットワークパス</h4>
        </ElCol>
        <ElCol :span="10">
          <ElInput type="text" v-model="config.STORAGE_CIFSSERVER" @input="FixPath('STORAGE_CIFSSERVER')" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip v-if="config.STORAGE_CIFS === 'on'" :tabindex="-1" placement="top" content="NASのユーザー名を設定します" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>アカウント</h4>
        </ElCol>
        <ElCol :span="6">
          <ElInput type="text" name="account" v-model="config.STORAGE_CIFSUSER" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip v-if="config.STORAGE_CIFS === 'on'" :tabindex="-1" placement="top" content="NASのパスワードを設定します" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>パスワード</h4>
        </ElCol>
        <ElCol :span="6">
          <ElInput type="text" name="password" v-model="config.STORAGE_CIFSPASSWD" show-password />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip v-if="config.STORAGE_CIFS === 'on'" :tabindex="-1" placement="top" content="録画するPATHをstrftimeの書式指定で記述します。最後に拡張子が付加されます。" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>保存するPATH</h4>
        </ElCol>
        <ElCol :span="6">
          <ElInput type="text" v-model="config.STORAGE_CIFS_PATH" @input="FixPath('STORAGE_CIFS_PATH')" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip v-if="config.STORAGE_CIFS === 'on'" :tabindex="-1" placement="top" content="CIFS Serverに録画したファイルを自動的に削除します" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>ファイルの自動削除</h4>
        </ElCol>
        <ElCol :span="3">
          <ElSwitch v-model="config.STORAGE_CIFS_REMOVE" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip v-if="config.STORAGE_CIFS === 'on' && config.STORAGE_CIFS_REMOVE === 'on'" :tabindex="-1" placement="top" content="指定日数後に削除します" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>保存日数</h4>
        </ElCol>
        <ElCol :span="3">
          <ElInputNumber v-model="config.STORAGE_CIFS_REMOVE_DAYS" :min="1" :step-strictly="true" size="mini" />
        </ElCol>
      </ElRow>
      </ElTooltip>

      <h3>ストリーミング</h3>
      <ElTooltip :tabindex="-1" placement="top" content="RTSPサーバーを起動します" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="1" :span="8">
            <h4>RTSP</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.RTSPSERVER" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip v-if="config.RTSPSERVER === 'on'" :tabindex="-1" placement="top" content="RTSPの音声を設定します" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>音声</h4>
        </ElCol>
        <ElCol :span="10">
          <ElSwitch v-model="config.RTSP_AUDIO" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElRow v-if="config.RTSPSERVER === 'on'">
        <ElCol :offset="2" :span="7">
          <h4>URL</h4>
        </ElCol>
        <ElCol :span="10">
          <ElInput type="text" readonly v-model="RTSP_URL" />
        </ElCol>
      </ElRow>

      <h3>イベント通知</h3>
      <ElTooltip :tabindex="-1" placement="top" content="WebHookを設定します" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="1" :span="8">
            <h4>WebHook</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      </ElTooltip>
      <ElTooltip v-if="config.WEBHOOK === 'on'" :tabindex="-1" placement="top" content="WebHookのイベント発生時にpostするURL" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>通知URL</h4>
        </ElCol>
        <ElCol :span="10">
          <ElInput type="text" v-model="config.WEBHOOK_URL" />
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip v-if="config.WEBHOOK === 'on'" :tabindex="-1" placement="top" content="Alarm発生を通知します" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>動体検知</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALARM_EVENT" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALARM_EVENT === 'on'" :span="4">
          type: alarmEvent
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip v-if="config.WEBHOOK === 'on'" :tabindex="-1" placement="top" content="Alarm発生時に認識情報を通知します" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>動体認識情報</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALARM_INFO" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALARM_INFO === 'on'" :span="4">
          type: recognitionNotify
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip v-if="config.WEBHOOK === 'on'" :tabindex="-1" placement="top" content="Alarm発生時に録画保存を通知します" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>動体検知録画終了</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALARM_VIDEO_FINISH" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALARM_VIDEO_FINISH === 'on'" :span="4">
          type: uploadVideoFinish
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="Alarm発生時に録画ファイルを転送します" effect="light" :open-delay="500">
      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
            <h4>動体検知録画転送</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALERM_VIDEO" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALERM_VIDEO === 'on'" :span="4">
          mime: video/mp4
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="Alarm発生時に静止画保存を通知します" effect="light" :open-delay="500">
      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
            <h4>動体検知静止画保存</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALARM_PICT_FINISH" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALARM_PICT_FINISH === 'on'" :span="4">
          type: uploadPictureFinish
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="Alarm発生時に静止画ファイルを転送します" effect="light" :open-delay="500">
      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
            <h4>動体検知静止画転送</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALERM_PICT" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALERM_PICT === 'on'" :span="4">
          mime: image/jpeg
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="録画保存を通知します" effect="light" :open-delay="500">
      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
            <h4>定常録画保存</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_RECORD_EVENT" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_RECORD_EVENT === 'on'" :span="4">
          type: recordEvent
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="タイムラプスイベントを通知します" effect="light" :open-delay="500">
      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
            <h4>タイムラプス記録</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_TIMELAPSE_EVENT" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_TIMELAPSE_EVENT === 'on'" :span="4">
          type: timelapseEvent
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="タイムラプス録画完了を通知します" effect="light" :open-delay="500">
      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
            <h4>タイムラプス録画完了</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_TIMELAPSE_FINISH" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_TIMELAPSE_FINISH === 'on'" :span="4">
          type: timelapseFinish
        </ElCol>
      </ElRow>
      </ElTooltip>

      <h3>動体検知</h3>
      <ElTooltip :tabindex="-1" placement="top" content="Alarmの無検知時間5分を30秒に短縮します" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>動体検知周期の短縮</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.MINIMIZE_ALARM_CYCLE" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol :span="6">
          ※ 変更すると設定ボタンで再起動します
        </ElCol>
      </ElRow>
      </ElTooltip>

      <h3>メンテナンス</h3>
      <ElTooltip v-if="isSwing" :tabindex="-1" placement="top" content="Swingの座標を両側の端点当てで修正します" effect="light" :open-delay="500">
        <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>Swing座標初期化</h4>
        </ElCol>
        <ElCol :span="4">
          <ElButton @click="MoveInit" type="primary" size="mini">初期化</ElButton>
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="定期的に再起動する設定をします" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>定期リスタート</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.REBOOT" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      </ElTooltip>

      <div v-if="config.REBOOT === 'on'">
        <ElRow>
          <ElCol :offset="9" :span="12">
            <div class="well schedule">
              <div class="schedule-week schedule-time">
                <ElCheckboxGroup v-model="reboot.dayOfWeekSelect" size="small" :min="1">
                  <ElCheckboxButton v-for="day of weekDays" :label="day" :key="day">
                    {{ day }}
                  </ElCheckboxButton>
                </ElCheckboxGroup>
              </div>
              <div class="schedule-week schedule-time">
                <ElTimePicker class="time-picker" v-model="reboot.time" placeholder="設定時間" value-format="HH:mm" format="HH:mm" />
              </div>
            </div>
          </ElCol>
        </ElRow>
      </div>

      <ElTooltip :tabindex="-1" placement="top" content="再起動します" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>リブート</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="rebootEnable" inactive-text="Lock" />
        </ElCol>
        <ElCol :span="4">
          <ElButton @click="DoReboot" type="danger" :disabled="!rebootEnable" icon="el-icon-refresh-left">Reboot</ElButton>
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="SD-Cardの録画フォルダを消去します" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>SD-Card消去</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="eraseEnable" inactive-text="Lock" />
        </ElCol>
        <ElCol :span="4">
          <ElButton @click="DoErase" type="danger" :disabled="!eraseEnable" icon="el-icon-folder-delete">Erase</ElButton>
        </ElCol>
      </ElRow>
      </ElTooltip>

      <ElTooltip :tabindex="-1" placement="top" content="このtoolのupdateをします" effect="light" :open-delay="500">
      <ElRow>
        <ElCol :offset="2" :span="7">
            <h4>Update</h4>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="updateEnable" inactive-text="Lock" :disabled="!updatable" />
        </ElCol>
        <ElCol :span="4">
          <ElButton @click="DoUpdate" type="danger" :disabled="!updateEnable" icon="el-icon-refresh">Update</ElButton>
        </ElCol>
        <ElCol :span="7">
          <span class="latest" :class="{ 'latest-updatable': updatable }">Latest Version : Ver.{{ latestVer }}</span>
        </ElCol>
      </ElRow>
      </ElTooltip>
    </div>
    <ElRow class="submit">
      <ElCol :offset="20" :span="4">
        <ElButton @click="Submit" type="primary" :disabled="stillFullView">設定</ElButton>
      </ElCol>
    </ElRow>
    <ElDrawer title="設定中" :visible.sync="executing" direction="btt" :show-close="false" :wrapperClosable="false">
      <h4 class="comment">設定変更中。暫くお待ち下さい。</h4>
    </ElDrawer>
    <ElDrawer title="再起動中" :visible.sync="rebooting" direction="btt" :show-close="false" :wrapperClosable="false">
      <h4 class="comment">起動するまで暫くお待ち下さい。(約{{ rebootTime }}秒)</h4>
    </ElDrawer>
  </div>
</template>

<script>
  import axios from 'axios';
  import md5 from 'js-md5';
  import { Tooltip, Switch, Input, InputNumber, CheckboxGroup, CheckboxButton, TimePicker, Drawer, Slider } from 'element-ui';
  import 'element-ui/lib/theme-chalk/tooltip.css';
  import 'element-ui/lib/theme-chalk/switch.css';
  import 'element-ui/lib/theme-chalk/input.css';
  import 'element-ui/lib/theme-chalk/input-number.css';
  import 'element-ui/lib/theme-chalk/checkbox.css';
  import 'element-ui/lib/theme-chalk/time-picker.css';
  import 'element-ui/lib/theme-chalk/drawer.css';
  import 'element-ui/lib/theme-chalk/slider.css';

  export default {
    name: 'ATOMCamSetting',
    components: {
      ElTooltip: Tooltip,
      ElSwitch: Switch,
      ElInput: Input,
      ElInputNumber: InputNumber,
      ElCheckboxGroup: CheckboxGroup,
      ElCheckboxButton: CheckboxButton,
      ElTimePicker: TimePicker,
      ElDrawer: Drawer,
      ElSlider: Slider,
    },
    data() {
      return {
        RTSP_URL: '',
        config: {
          appver: '', // ATOMCam app_ver (/atom/config/app.ver)
          ATOMHACKVER: '', // AtomHack Ver (/etc/atomhack.ver)
          PRODUCT_MODEL: '', // ATOMCam Model (/atom/configs/.product_config)
          HOSTNAME: 'atomcam', // ATOMHack hostname (/media/mmc/hostname)
          DIGEST: '',
          REBOOT: 'off',
          REBOOT_SCHEDULE: '0 2 * * 7', // -> /var/spool/crontabs/root
          RECORDING_ALARM: 'on',
          RECORDING_LOCAL_SCHEDULE: 'off',
          RECORDING_LOCAL_SCHEDULE_LIST: '', // -> /media/mmc/local_schedule
          RTSPSERVER: 'off',
          RTSP_AUDIO: 'off',
          STORAGE_SDCARD: 'on',
          STORAGE_SDCARD_PUBLISH: 'off',
          STORAGE_SDCARD_PATH: '%Y%m%d/%H%M%S',
          STORAGE_SDCARD_REMOVE: 'off',
          STORAGE_SDCARD_REMOVE_DAYS: 30,
          STORAGE_CIFS: 'off',
          STORAGE_CIFS_PATH: '%Y%m%d/%H%M%S',
          STORAGE_CIFS_REMOVE: 'off',
          STORAGE_CIFS_REMOVE_DAYS: 30,
          STORAGE_CIFSSERVER: '',
          STORAGE_CIFSUSER: '',
          STORAGE_CIFSPASSWD: '',
          WEBHOOK: 'off',
          WEBHOOK_URL: '',
          WEBHOOK_ALARM_EVENT: 'off',
          WEBHOOK_ALARM_INFO: 'off',
          WEBHOOK_ALARM_VIDEO_FINISH: 'off',
          WEBHOOK_ALERM_VIDEO: 'off',
          WEBHOOK_ALARM_PICT_FINISH: 'off',
          WEBHOOK_ALERM_PICT: 'off',
          WEBHOOK_RECORD_EVENT: 'off',
          WEBHOOK_TIMELAPSE_EVENT: 'off',
          WEBHOOK_TIMELAPSE_FINISH: 'off',
          MINIMIZE_ALARM_CYCLE: 'off',
        },
        loginAuth: 'off',
        relm: 'atomcam',
        account: '',
        password: '',
        intervalValue: {
          TIMESTAMP: '',
        },
        schedule: [],
        reboot: {
          time: '02:00',
          dayOfWeekSelect: ['日'],
        },
        rebootEnable: false,
        rebootTime: 80,
        updateEnable: false,
        latestVer: '',
        eraseEnable: false,
        executing: false,
        rebooting: false,
        stillCount: 0,
        stillFullView: false,
        weekDays: ['月','火', '水', '木', '金', '土', '日'],
        pan: 0,
        tilt: 0,
        posValid: false,
      };
    },
    computed: {
      stillInterval() {
        return this.stillFullView ? 500 : 1000;
      },
      imageFrameStyle() {
        return this.stillFullView ? { right: '10px', width: '98%' } : { right: '30px', width: '30%' };
      },
      updatable() {
        const ver = this.config.ATOMHACKVER.split('.');
        if(ver.length !== 3) return false;
        const latest = this.latestVer.split('.');
        if(latest.length !== 3) return false;
        if(parseInt(ver[0]) < parseInt(latest[0])) return true;
        if(parseInt(ver[0]) > parseInt(latest[0])) return false;
        if(parseInt(ver[1]) < parseInt(latest[1])) return true;
        if(parseInt(ver[1]) > parseInt(latest[1])) return false;
        if(parseInt(ver[2]) < parseInt(latest[2])) return true;
        return false;
      },
      isSwing() {
        return !this.rebooting && this.posValid && (this.config.PRODUCT_MODEL === 'ATOM_CAKP1JZJP');
      },
    },
    async mounted() {
      const res = await axios.get('./cgi-bin/hack_ini.cgi').catch(err => {
        // eslint-disable-next-line no-console
        console.log(err);
        return '';
      });

      this.oldConfig = res.data.split('\n').reduce((d, l) => {
        const name = l.split(/[ \t=]/)[0].trim();
        if(d[name] != null) d[name] = l.replace(new RegExp(name + '[ \t=]*'), '').trim();
        return d;
      }, Object.assign({}, this.config));
      this.config = Object.assign({}, this.oldConfig);
      // eslint-disable-next-line no-console
      console.log(this.config);

      if(this.config.DIGEST.length) {
        this.loginAuth = 'on';
        this.account = this.config.DIGEST.replace(/:.*$/, '');
      }

      if(this.config.RECORDING_LOCAL_SCHEDULE_LIST) {
        let index = -1;
        this.schedule = this.config.RECORDING_LOCAL_SCHEDULE_LIST.split(';').reduce((d, l) => {
          if(l.search(/\[index_.*\]/) >= 0) {
            index = l.replace(/^.*_(\d*).*$/, '$1') - 1;
            d[index] = {};
            return d;
          }
          const ll = l.split(/=/);
          if(ll[0] === 'Rule') {
            d[index].dayOfWeekSelect = [];
            this.weekDays.forEach((l, i) => (ll[1] & (2 << i)) && d[index].dayOfWeekSelect.push(l));
          }
          if(ll[0] === 'ContinueTime') d[index].continueTimeNum = parseInt(ll[1]);
          if(ll[0] === 'StartTime') d[index].startTimeNum = parseInt(ll[1]);
          if((d[index].continueTimeNum != null) && (d[index].startTimeNum != null)) {
            d[index].startTime = parseInt(d[index].startTimeNum / 60).toString().padStart(2, '0') + ':' + (d[index].startTimeNum % 60).toString().padStart(2, '0');
            const endTime = d[index].startTimeNum + d[index].continueTimeNum - 1;
            d[index].endTime = parseInt(endTime / 60).toString().padStart(2, '0') + ':' + (endTime % 60).toString().padStart(2, '0');
            delete(d[index].continueTimeNum);
            delete(d[index].startTimeNum);
          }
          return d;
        }, []);
      }

      const status = (await axios.get('./cgi-bin/cmd.cgi').catch(err => {
        // eslint-disable-next-line no-console
        console.log(err);
        return { data: '' };
      })).data.split('\n').reduce((s, d) => {
        s[d.replace(/=.*$/, '').trim()] = d.replace(/^.*=/, '').trim();
        return s;
      }, {});

      this.latestVer = status.LATESTVER;
      const pos = status.MOTORPOS.split(' ');
      this.pan = parseFloat(pos[0]);
      this.tilt = parseFloat(pos[1]);
      this.posValid = true;

      if(this.config.REBOOT_SCHEDULE) {
        const str = this.config.REBOOT_SCHEDULE.split(' ');
        const days = (str[4] || '').split(':');
        this.reboot.time = `${str[1].padStart(2, '0')}:${str[0].padStart(2, '0')}`;
        this.reboot.dayOfWeekSelect = days.map(d => this.weekDays[d - 1]);
      }

      this.RTSP_URL = `rtsp://${window.location.host}:8554/unicast`;

      setInterval(async () => {
        const res = await axios.get('./cgi-bin/cmd.cgi?name=time').catch(err => {
          // eslint-disable-next-line no-console
          console.log(err);
          return '';
        });
        if(res === '') return;
        if(this.rebootStart && (new Date() > this.rebootStart)) {
          this.rebooting = false;
          this.rebootStart = null;
          location.reload();
        }
        this.intervalValue = res.data.split('\n').reduce((d, l) => {
          const name = l.split(/[ \t=]/)[0].trim();
          if(name) d[name] = l.replace(new RegExp(name + '[ \t=]*'), '').trim();
          return d;
        }, {});
      }, 1000);
      this.StillImageInterval();
      if(this.isSwing) this.GetPosInterval();
    },
    methods: {
      async Move() {
        if(!this.posValid) return;
        this.Exec(`move ${this.pan} ${this.tilt}`, 'socket');
        this.StillImageInterval();
        if(this.moveTimeout) clearTimeout(this.moveTimeout);
        this.moveTimeout = setTimeout(() => {
          this.moveTimeout = null;
          this.Exec('posrec');
        }, 3000);
      },
      async GetPosInterval() {
        const status = (await axios.get('./cgi-bin/cmd.cgi?name=move').catch(err => {
          // eslint-disable-next-line no-console
          console.log(err);
          return { data: '' };
        })).data.split('\n').reduce((s, d) => {
          s[d.replace(/=.*$/, '').trim()] = d.replace(/^.*=/, '').trim();
          return s;
        }, {});
        if(status.MOTORPOS) {
          const pos = status.MOTORPOS.split(' ');
          this.pan = parseFloat(pos[0]);
          this.tilt = parseFloat(pos[1]);
        }
        if(this.getposTimeout) clearTimeout(this.getposTimeout);
        this.getposTimeout = setTimeout(this.GetPosInterval.bind(this), 1000);
      },
      StillImageInterval() {
        this.stillCount++;
        if(this.imageTimeout) clearTimeout(this.imageTimeout);
        this.imageTimeout = setTimeout(this.StillImageInterval.bind(this), this.stillInterval);
      },
      AddSchedule() {
        this.schedule.push({
          allDay: true,
          startTime: '08:00',
          endTime: '21:59',
          dayOfWeekSelect: this.weekDays.concat(),
        });
      },
      DeleteSchedule(i) {
        this.schedule.splice(i, 1);
        if(!this.schedule.length) this.config.RECORDING_LOCAL_SCHEDULE = false;
      },
      FixPath(label) {
        this.config[label] = this.config[label].replace(/\\/g, '/');
      },
      MoveInit() {
        this.Exec('moveinit');
      },
      DoReboot() {
        this.rebootTime = 80;
        this.rebooting = true;
        this.rebootStart = new Date();
        this.rebootStart.setSeconds(this.rebootStart.getSeconds() + 30);
        this.Exec('reboot');
      },
      DoErase() {
        this.executing = true;
        this.Exec('sderase');
        this.executing = false;
      },
      async DoUpdate() {
        this.rebootTime = 180;
        this.rebooting = true;
        this.rebootStart = new Date();
        this.rebootStart.setSeconds(this.rebootStart.getSeconds() + 180);
        await this.Exec('update');
        this.rebootStart = new Date();
        this.rebootStart.setSeconds(this.rebootStart.getSeconds() + 30);
      },
      async Submit() {
        if((this.loginAuth === 'on') && this.account.length) {
          if(this.password.length) {
            this.config.DIGEST = `${this.account}:${this.relm}:` + md5(`${this.account}:${this.relm}:${this.password}`);
          }
        } else {
          this.config.DIGEST='';
        }

        let str = '';
        for(const i in this.schedule) {
          const timeTable = this.schedule[i];
          str += `[index_${(i - 0 + 1).toString().padStart(2, '0')}];`;
          const val = this.weekDays.reduce((v, l, j) => v |= timeTable.dayOfWeekSelect.indexOf(l) >= 0 ? 2 << j : 0, 0);
          str += `Rule=${val};`;
          const stime = parseInt(timeTable.startTime.slice(0, 2)) * 60 + parseInt(timeTable.startTime.slice(-2));
          const etime = parseInt(timeTable.endTime.slice(0, 2)) * 60 + parseInt(timeTable.endTime.slice(-2)) + 1;
          str += `ContinueTime=${etime - stime};`;
          str += `StartTime=${stime};`;
          str += `Status=1;`;
          str += `DelFlags=1;`;
        }
        this.config.RECORDING_LOCAL_SCHEDULE_LIST = str;
        if(this.config.STORAGE_SDCARD !== "on") this.config.STORAGE_SDCARD_PUBLISH = "off";

        str = parseInt(this.reboot.time.slice(-2)) + ' ';
        str += parseInt(this.reboot.time.slice(0, 2)) + ' * * ';
        str += this.weekDays.reduce((s, v, d) => {
          if(this.reboot.dayOfWeekSelect.indexOf(v) < 0) return s;
          if(s.length) s += ':';
          s += d + 1;
          return s;
        }, '');
        this.config.REBOOT_SCHEDULE = str;

        await axios.post('./cgi-bin/hack_ini.cgi', this.config).catch(err => {
          // eslint-disable-next-line no-console
          console.log(err);
        });

        const execCmds = [];
        let href = null;
        if(this.config.REBOOT_SCHEDULE !== this.oldConfig.REBOOT_SCHEDULE) {
          execCmds.push(`scheduleReboot ${this.config.REBOOT_SCHEDULE}`);
        }
        if(this.config.HOSTNAME !== this.oldConfig.HOSTNAME) {
          execCmds.push(`hostname ${this.config.HOSTNAME}`);
          if(window.location.host === `${this.oldConfig.HOSTNAME}.local`) {
            href = `http://${this.config.HOSTNAME}.local`;
          }
        }
        if(this.config.MINIMIZE_ALARM_CYCLE !== this.oldConfig.MINIMIZE_ALARM_CYCLE) {
          execCmds.push(`reboot`);
          this.rebooting = true;
          this.rebootStart = new Date();
          this.rebootStart.setSeconds(this.rebootStart.getSeconds() + 30);
        } else {
          if((this.config.RTSPSERVER !== this.oldConfig.RTSPSERVER) && (this.config.RTSPSERVER === "off")) {
            execCmds.push(`rtspserver ${this.config.RTSPSERVER}`);
          }
          if(this.config.STORAGE_SDCARD_PUBLISH !== this.oldConfig.STORAGE_SDCARD_PUBLISH) {
            execCmds.push(`samba ${this.config.STORAGE_SDCARD_PUBLISH}`);
          }
          if(((this.config.RTSPSERVER !== this.oldConfig.RTSPSERVER) ||
              (this.config.RTSP_AUDIO !== this.oldConfig.RTSP_AUDIO)) &&
              (this.config.RTSPSERVER === "on")) {
            execCmds.push(`rtspserver ${this.config.RTSPSERVER}`);
          }
          if(Object.keys(this.config).some(prop => (prop.search(/WEBHOOK/) === 0) && (this.config[prop] !== this.oldConfig[prop]))) {
            execCmds.push('setwebhook');
          }
        }
        if(this.config.DIGEST !== this.oldConfig.DIGEST) execCmds.push('lighttpd');

        this.oldConfig = Object.assign({}, this.config);
        if(execCmds.length) {
          this.executing = true;
          this.$nextTick(async () => {
            for(const cmd of execCmds) {
              if(cmd === 'reboot') {
                this.DoReboot();
                break;
              }
              await this.Exec(cmd);
            }
            if(execCmds.indexOf('lighttpd') >= 0) {
              setTimeout(() => this.executing = false, 3000);
            } else {
            this.executing = false;
            }
            if(href) window.location.href = href;
          });
        }
      },
      async Exec(cmd, port) {
        return await axios.post(`./cgi-bin/cmd.cgi?port=${port}`, { exec: cmd }).catch(err => {
          // eslint-disable-next-line no-console
          console.log(err);
        });
      },
    },
  };
</script>

<style scoped>
  .title {
    display: flex;
    justify-content: space-between;
    align-items: flex-end;
    font-size: 2.5rem;
    padding: 0 15px 5px 30px;
    color: white;
    background-color: #bc423a;
    height:60px;
  }

  .version {
    font-size: 1.5rem;
  }

  .atomcam {
    font-size: 1.5rem;
    flex-grow: 1;
    text-align: right;
    padding-right: 16px;
  }

  .timestamp {
    font-size: 1.5rem;
    flex-basis: 15rem;
  }

  .help {
    font-size: 30px;
    float:right;
    color: snow;
  }

  .container {
    height: calc(100vh - 160px);
    margin: 10px 20px 5px 20px;
    overflow-x: hidden;
    overflow-y: scroll;
  }

  .image-frame {
    z-index: 100;
    display: flex;
    position: fixed;
    flex-direction: column;
  }

  .image-frame-inner1 {
    justify-content: flex-end;
    display: flex;
  }

  .image-frame-inner2 {
    justify-content: flex-end;
    display: flex;
  }

  .still-image {
    width: calc(100% - 36px);
  }

  .pan-slider {
    background-color: white;
    width: calc(100% - 36px);
  }

  .tilt-slider {
    background-color: white;
    align-items: stretch;
  }

  .environment {
    margin: 5px 30px 2px 30px;
  }
  .schedule {
    display: inline-flex;
    flex-direction: column;
    width: 95%;
  }
  .schedule-week {
    display: inline-flex;
    justify-content: space-between;
  }
  .schedule-time {
    display: inline-flex;
    margin:10px 5px 5px 5px;
  }
  .schedule-timerange {
    margin:0px 0px 0px 10px;
  }
  .schedule-button {
    padding: 0;
    text-alin:right;
    float:right;
  }
  .time-picker {
    width: 110px;
  }
  .submit {
    margin: 20px 0 20px 0;
  }
  .comment {
    width: 100%;
    margin: 30px;
    text-align: center;
  }
  .latest {
    font-size: 1.2em;
    font-weight: 300;
  }
  .latest-updatable {
    color: 'red';
    font-size: 1.2em;
    font-weight: 600;
  }
</style>
<style>
  .el-checkbox-button.is-disabled.is-checked .el-checkbox-button__inner {
    color: #C0C4CC;
    cursor: not-allowed;
    background-image: none;
    background-color: #409EFF;
    border-color: #409EFF;
    -webkit-box-shadow: none;
    box-shadow: none;
  }
</style>
