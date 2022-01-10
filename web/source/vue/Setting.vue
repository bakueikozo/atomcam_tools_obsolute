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
        {{ intervalValue.TIMESTAMP }}
      </span>
    </div>
    <div class="well-transparent container">
      <div class="image-frame" :style="imageFrameStyle">
        <ElTooltip :tabindex="-1" placement="top" :content="stillFullView?'clickで縮小します':'clickで拡大します'" effect="light" :open-delay="500">
          <img class="still-image" :src="`/cgi-bin/get_jpeg.cgi?r=${stillCount}`" @click="stillFullView=!stillFullView">
        </ElTooltip>
      </div>
      <h3>基本設定</h3>
      <ElRow>
        <ElCol :offset="1" :span="8">
          <ElTooltip :tabindex="-1" placement="top" content="NASの保存フォルダ名やリモートアクセスのための名前を設定します" effect="light" :open-delay="500">
            <h4>デバイス名</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElInput type="text" v-model="config.HOSTNAME" />
        </ElCol>
      </ElRow>

      <h3>録画</h3>
      <ElRow>
        <ElCol :offset="1" :span="8">
          <ElTooltip :tabindex="-1" placement="top" content="モーション／サウンド検出したときの12秒の映像をローカルにも録画します" effect="light" :open-delay="500">
            <h4>検出通知のローカル録画</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="3">
          <ElSwitch v-model="config.RECORDING_ALARM" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>

      <ElRow>
        <ElCol :offset="1" :span="8">
          <ElTooltip :tabindex="-1" placement="top" content="SD-Card/NASに録画される時間帯を設定します。" effect="light" :open-delay="500">
            <h4>ローカル録画スケジュール</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="10">
          <ElSwitch v-model="config.RECORDING_LOCAL_SCHEDULE" active-value="on" inactive-value="off" active-color="#13ce66" inactive-color="#409eff" active-text="スケジュール" inactive-text="常時録画" @change="config.RECORDING_LOCAL_SCHEDULE && !schedule.length && AddSchedule()" />
        </ElCol>
      </ElRow>
      <div v-if="config.RECORDING_LOCAL_SCHEDULE === 'on'">
        <ElRow v-for="(timeTable, idx) of schedule" :key="'timetable'+idx">
          <ElCol :offset="9" :span="12">
            <div class="well schedule">
              <div class="schedule-week">
                <ElCheckboxGroup v-model="timeTable.dayOfWeekSelect" size="small">
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
      <ElRow>
        <ElCol :offset="1" :span="8">
          <ElTooltip :tabindex="-1" placement="top" content="SD-Card/CIFS Serverに録画したファイルを自動的に削除します" effect="light" :open-delay="500">
            <h4>録画ファイルの自動削除</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="3">
          <ElSwitch v-model="config.STORAGE_REMOVE" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      <ElRow v-if="config.STORAGE_REMOVE === 'on'">
        <ElCol :offset="1" :span="8">
          <ElTooltip :tabindex="-1" placement="top" content="指定日数後に削除します" effect="light" :open-delay="500">
            <h4>保存日数</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="3">
          <ElInputNumber v-model="config.STORAGE_REMOVE_DAYS" :min="1" :step-strictly="true" size="mini" />
        </ElCol>
      </ElRow>
      <ElRow>
        <ElCol :offset="1" :span="8">
          <ElTooltip :tabindex="-1" placement="top" content="録画するPATHをstrftimeの書式指定で記述します。SD-Cardの録画ファイルには効きません。" effect="light" :open-delay="500">
            <h4>録画ファイルのPATH</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="6">
          <ElInput type="text" v-model="config.RECORDING_PATH" />
        </ElCol>
      </ElRow>

      <h3>記録メディア</h3>
      <ElRow>
        <ElCol :offset="1" :span="8">
          <ElTooltip :tabindex="-1" placement="top" content="SD-Cardへの記録をします" effect="light" :open-delay="500">
            <h4>SD-Card</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.STORAGE_SDCARD" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      <ElRow v-if="config.STORAGE_SDCARD === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="ATOMCamのSD-CardをCIFS(smb) serverとしてLAN内に公開します" effect="light" :open-delay="500">
            <h4>ネットワークアクセス</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.STORAGE_SDCARD_PUBLISH" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>

      <ElRow>
        <ElCol :offset="1" :span="8">
          <ElTooltip :tabindex="-1" placement="top" content="NAS(CIFS Server)への記録をします" effect="light" :open-delay="500">
            <h4>NAS</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.STORAGE_CIFS" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>

      <ElRow v-if="config.STORAGE_CIFS === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="NASのパスを設定。(//server/folder/の形式で指定)" effect="light" :open-delay="500">
            <h4>ネットワークパス</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="10">
          <ElInput type="text" v-model="config.STORAGE_CIFSSERVER" />
        </ElCol>
      </ElRow>
      <ElRow v-if="config.STORAGE_CIFS === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="NASのユーザー名を設定します" effect="light" :open-delay="500">
            <h4>アカウント</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="6">
          <ElInput type="text" name="account" v-model="config.STORAGE_CIFSUSER" />
        </ElCol>
      </ElRow>
      <ElRow v-if="config.STORAGE_CIFS === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="NASのパスワードを設定します" effect="light" :open-delay="500">
            <h4>パスワード</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="6">
          <ElInput type="text" name="password" v-model="config.STORAGE_CIFSPASSWD" show-password />
        </ElCol>
      </ElRow>

      <h3>ストリーミング</h3>
      <ElRow>
        <ElCol :offset="1" :span="8">
          <ElTooltip :tabindex="-1" placement="top" content="RTSPサーバーを起動します" effect="light" :open-delay="500">
            <h4>RTSP</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.RTSPSERVER" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      <ElRow v-if="config.RTSPSERVER === 'on'">
        <ElCol :offset="2" :span="7">
          <h4>URL</h4>
        </ElCol>
        <ElCol :span="10">
          <ElInput type="text" readonly v-model="RTSP_URL" />
        </ElCol>
      </ElRow>

      <h3>イベント通知</h3>
      <ElRow>
        <ElCol :offset="1" :span="8">
          <ElTooltip :tabindex="-1" placement="top" content="WebHookを設定します" effect="light" :open-delay="500">
            <h4>WebHook</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>
      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="WebHookのイベント発生時にpostするURL" effect="light" :open-delay="500">
            <h4>通知URL</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="10">
          <ElInput type="text" v-model="config.WEBHOOK_URL" />
        </ElCol>
      </ElRow>

      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="Alarm発生を通知します" effect="light" :open-delay="500">
            <h4>動体検知</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALARM_EVENT" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALARM_EVENT === 'on'" :span="4">
          type: alarmEvent
        </ElCol>
      </ElRow>

      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="Alarm発生時に認識情報を通知します" effect="light" :open-delay="500">
            <h4>動体認識情報</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALARM_INFO" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALARM_INFO === 'on'" :span="4">
          type: recognitionNotify
        </ElCol>
      </ElRow>

      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="Alarm発生時に録画保存を通知します" effect="light" :open-delay="500">
            <h4>動体検知録画終了</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALARM_VIDEO_FINISH" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALARM_VIDEO_FINISH === 'on'" :span="4">
          type: uploadVideoFinish
        </ElCol>
      </ElRow>

      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="Alarm発生時に録画ファイルを転送します" effect="light" :open-delay="500">
            <h4>動体検知録画転送</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALERM_VIDEO" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALERM_VIDEO === 'on'" :span="4">
          mime: video/mp4
        </ElCol>
      </ElRow>

      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="Alarm発生時に静止画保存を通知します" effect="light" :open-delay="500">
            <h4>動体検知静止画保存</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALARM_PICT_FINISH" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALARM_PICT_FINISH === 'on'" :span="4">
          type: uploadPictureFinish
        </ElCol>
      </ElRow>

      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="Alarm発生時に静止画ファイルを転送します" effect="light" :open-delay="500">
            <h4>動体検知静止画転送</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_ALERM_PICT" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_ALERM_PICT === 'on'" :span="4">
          mime: image/jpeg
        </ElCol>
      </ElRow>

      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="録画保存を通知します" effect="light" :open-delay="500">
            <h4>定常録画保存</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_RECORD_EVENT" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_RECORD_EVENT === 'on'" :span="4">
          type: recordEvent
        </ElCol>
      </ElRow>

      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="タイムラプスイベントを通知します" effect="light" :open-delay="500">
            <h4>タイムラプス記録</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_TIMELAPSE_EVENT" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_TIMELAPSE_EVENT === 'on'" :span="4">
          type: timelapseEvent
        </ElCol>
      </ElRow>

      <ElRow v-if="config.WEBHOOK === 'on'">
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="タイムラプス録画完了を通知します" effect="light" :open-delay="500">
            <h4>タイムラプス録画完了</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.WEBHOOK_TIMELAPSE_FINISH" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol v-if="config.WEBHOOK_TIMELAPSE_FINISH === 'on'" :span="4">
          type: timelapseFinish
        </ElCol>
      </ElRow>

      <h3>動体検知</h3>
      <ElRow>
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="Alarmの無検知時間5分を30秒に短縮します" effect="light" :open-delay="500">
            <h4>動体検知周期の短縮</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.MINIMIZE_ALARM_CYCLE" active-value="on" inactive-value="off" />
        </ElCol>
        <ElCol :span="6">
          ※ 変更すると設定ボタンで再起動します
        </ElCol>
      </ElRow>

      <h3>メンテナンス</h3>
      <ElRow>
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="定期的に再起動する設定をします" effect="light" :open-delay="500">
            <h4>定期リスタート</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="config.REBOOT" active-value="on" inactive-value="off" />
        </ElCol>
      </ElRow>

      <div v-if="config.REBOOT === 'on'">
        <ElRow>
          <ElCol :offset="9" :span="12">
            <div class="well schedule">
              <div class="schedule-week schedule-time">
                <ElCheckboxGroup v-model="reboot.dayOfWeekSelect" size="small">
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

      <ElRow>
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="再起動します" effect="light" :open-delay="500">
            <h4>リブート</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="rebootEnable" inactive-text="Lock" />
        </ElCol>
        <ElCol :span="4">
          <ElButton @click="DoReboot" type="danger" :disabled="!rebootEnable" icon="el-icon-refresh-left">Reboot</ElButton>
        </ElCol>
      </ElRow>

      <ElRow>
        <ElCol :offset="2" :span="7">
          <ElTooltip :tabindex="-1" placement="top" content="SD-Cardの録画フォルダを消去します" effect="light" :open-delay="500">
            <h4>SD-Card消去</h4>
          </ElTooltip>
        </ElCol>
        <ElCol :span="4">
          <ElSwitch v-model="eraseEnable" inactive-text="Lock" />
        </ElCol>
        <ElCol :span="4">
          <ElButton @click="DoErase" type="danger" :disabled="!eraseEnable" icon="el-icon-folder-delete">Erase</ElButton>
        </ElCol>
      </ElRow>
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
      <h4 class="comment">起動するまで暫くお待ち下さい。(約80秒)</h4>
    </ElDrawer>
  </div>
</template>

<script>
  import axios from 'axios';
  import { Tooltip, Switch, Input, InputNumber, CheckboxGroup, CheckboxButton, TimePicker, Drawer } from 'element-ui';
  import 'element-ui/lib/theme-chalk/tooltip.css';
  import 'element-ui/lib/theme-chalk/switch.css';
  import 'element-ui/lib/theme-chalk/input.css';
  import 'element-ui/lib/theme-chalk/input-number.css';
  import 'element-ui/lib/theme-chalk/checkbox.css';
  import 'element-ui/lib/theme-chalk/time-picker.css';
  import 'element-ui/lib/theme-chalk/drawer.css';

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
    },
    data() {
      return {
        RTSP_URL: '',
        config: {
          appver: '', // ATOMCam app_ver (/atom/config/app.ver)
          ATOMHACKVER: '', // AtomHack Ver (/etc/atomhack.ver)
          PRODUCT_MODEL: '', // ATOMCam Model (/atom/configs/.product_config)
          HOSTNAME: 'atomcam', // ATOMHack hostname (/media/mmc/hostname)
          REBOOT: 'off',
          REBOOT_SCHEDULE: '0 2 * * 7', // -> /var/spool/crontabs/root
          RECORDING_ALARM: 'on',
          RECORDING_LOCAL_SCHEDULE: 'off',
          RECORDING_LOCAL_SCHEDULE_LIST: '', // -> /media/mmc/local_schedule
          RECORDING_PATH: '%Y%m%d/%H%M%S',
          RTSPSERVER: 'off',
          STORAGE_CIFS: 'off',
          STORAGE_REMOVE: 'off',
          STORAGE_REMOVE_DAYS: 30,
          STORAGE_CIFSSERVER: '',
          STORAGE_CIFSUSER: '',
          STORAGE_CIFSPASSWD: '',
          STORAGE_SDCARD: 'on',
          STORAGE_SDCARD_PUBLISH: 'off',
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
        intervalValue: {
          TIMESTAMP: '',
        },
        schedule: [],
        reboot: {
          time: '02:00',
          dayOfWeekSelect: ['日'],
        },
        rebootEnable: false,
        eraseEnable: false,
        executing: false,
        rebooting: false,
        stillCount: 0,
        stillFullView: false,
        weekDays: ['月','火', '水', '木', '金', '土', '日'],
      };
    },
    computed: {
      stillInterval() {
        return this.stillFullView ? 500 : 1000;
      },
      imageFrameStyle() {
        return this.stillFullView ? { right: '10px', width: '98%' } : { right: '30px', width: '30%' };
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

      if(this.config.REBOOT_SCHEDULE) {
        const str = this.config.REBOOT_SCHEDULE.split(' ');
        const days = (str[4] || '').split(',');
        this.reboot.time = `${str[1].padStart(2, '0')}:${str[0].padStart(2, '0')}`;
        this.reboot.dayOfWeekSelect = days.map(d => this.weekDays[d - 1]);
      }

      this.RTSP_URL = `rtsp://${window.location.host}:8554/unicast`;

      setInterval(async () => {
        const res = await axios.get('./cgi-bin/get_time.cgi').catch(err => {
          // eslint-disable-next-line no-console
          console.log(err);
          return '';
        });
        if(res === '') return;
        if(new Date() - this.rebootStart > 10 * 1000) this.rebooting = false;
        this.intervalValue = res.data.split('\n').reduce((d, l) => {
          const name = l.split(/[ \t=]/)[0].trim();
          if(name) d[name] = l.replace(new RegExp(name + '[ \t=]*'), '').trim();
          return d;
        }, {});
      }, 1000);
      this.StillImageInterval();
    },
    methods: {
      StillImageInterval() {
        this.stillCount++;
        setTimeout(this.StillImageInterval.bind(this), this.stillInterval);
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
      DoReboot() {
        setTimeout(() => { location.reload(); }, 80000);
        this.rebooting = true;
        this.rebootStart = new Date();
        this.Exec('reboot');
      },
      DoErase() {
        this.executing = true;
        this.Exec('sderase');
        this.executing = false;
      },
      async Submit() {
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
        str += this.reboot.dayOfWeekSelect.reduce((s, v) => {
          const d = this.weekDays.indexOf(v) + 1;
          if(d <= 0) return s;
          if(s.length) s += ':';
          s += d;
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
          setTimeout(() => { location.reload(); }, 80000);
          this.rebooting = true;
          this.rebootStart = new Date();
        } else {
          if((this.config.RTSPSERVER !== this.oldConfig.RTSPSERVER) && (this.config.RTSPSERVER === "off")) {
            execCmds.push(`rtspserver ${this.config.RTSPSERVER}`);
          }
          if(this.config.STORAGE_SDCARD_PUBLISH !== this.oldConfig.STORAGE_SDCARD_PUBLISH) {
            execCmds.push(`samba ${this.config.STORAGE_SDCARD_PUBLISH}`);
          }
          if((this.config.RTSPSERVER !== this.oldConfig.RTSPSERVER) && (this.config.RTSPSERVER === "on")) {
            execCmds.push(`rtspserver ${this.config.RTSPSERVER}`);
          }
          if(Object.keys(this.config).some(prop => (prop.search(/WEBHOOK/) === 0) && (this.config[prop] !== this.oldConfig[prop]))) {
            execCmds.push('setwebhook');
          }
        }

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
            this.executing = false;
            if(href) window.location.href = href;
          });
        }
      },
      async Exec(cmd) {
        await axios.post('./cgi-bin/exec.cgi', { exec: `${cmd}` }).catch(err => {
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
    width: 45%;
  }

  .container {
    height: calc(100vh - 160px);
    margin: 10px 20px 5px 20px;
    overflow-x: hidden;
    overflow-y: scroll;
  }

  .image-frame {
    z-index: 100;
    display: block;
    position: fixed;
  }

  .still-image {
    width: 100%;
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
</style>
