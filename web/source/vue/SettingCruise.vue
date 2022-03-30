<template>
  <ElRow>
    <ElCol :offset="9" :span="14" @click.native.stop="$emit('click')">
      <div class="well cruise" :class="{ selected: selected }">
        <ElTooltip :tabindex="-1" placement="top" content="カメラの向きを設定。jpeg windowでも調整できます" effect="light" :open-delay="500">
          <ElRow>
            <ElCol :span="10">
              pan
              <ElInputNumber v-model="innerValue.pan" :min="0" :max="355" :step="5" :precision="0" size="mini" @input="Pan" />
            </ElCol>
            <ElCol :span="10">
              tilt
              <ElInputNumber v-model="innerValue.tilt" :min="0" :max="180" :step="5" :precision="0" size="mini" @input="Tilt" />
            </ElCol>
            <ElCol :offset="3" :span="1">
              <ElButton class="list-button" type="text" size="small" circle icon="el-icon-minus" @click="$emit('remove', $event)" />
            </ElCol>
          </ElRow>
        </ElTooltip>
        <ElRow>
          <ElTooltip :tabindex="-1" placement="top" content="カメラ動作後の待機時間を設定します" effect="light" :open-delay="500">
            <ElCol :span="14">
              動作後待機時間
              <ElInputNumber v-model="innerValue.wait" :min="10" :step="5" :precision="0" size="mini" @input="Submit" />
              秒
            </ElCol>
          </ElTooltip>
          <ElCol :span="9">
            <ElTooltip :tabindex="-1" placement="top" content="待機中に動体検知すると待機時間を延長します" effect="light" :open-delay="500">
              <ElCheckbox v-model="innerValue.detect" @change="Submit">検知</ElCheckbox>
            </ElTooltip>
            <ElTooltip :tabindex="-1" placement="top" content="待機中に動体検知すると追尾し、待機時間を延長します" effect="light" :open-delay="500">
              <ElCheckbox v-model="innerValue.follow" :disabled="!innerValue.detect" @change="Submit">追尾</ElCheckbox>
            </ElTooltip>
          </ElCol>
          <ElCol v-if="!innerValue.detect" :span="1">
            <ElButton class="list-button" type="text" size="small" circle icon="el-icon-plus" @click.stop="$emit('add', $event)" />
          </ElCol>
        </ElRow>
        <ElRow v-if="innerValue.detect">
          <ElTooltip :tabindex="-1" placement="top" content="動体検知終了後の待機時間を設定します" effect="light" :open-delay="500">
            <ElCol :span="14">
              検知後待機時間
              <ElInputNumber v-model="innerValue.timeout" :min="10" :step="5" :precision="0" size="mini" @input="Submit" />
              秒
            </ElCol>
          </ElTooltip>
          <ElCol :offset="9" :span="1">
            <ElButton class="list-button" type="text" size="small" circle icon="el-icon-plus" @click.stop="$emit('add', $event)" />
          </ElCol>
        </ElRow>
      </div>
    </ElCol>
  </ElRow>
</template>

<script>
  import { Tooltip, InputNumber, Checkbox } from 'element-ui';
  import 'element-ui/lib/theme-chalk/tooltip.css';
  import 'element-ui/lib/theme-chalk/switch.css';
  import 'element-ui/lib/theme-chalk/checkbox.css';
  import 'element-ui/lib/theme-chalk/input-number.css';

  export default {
    components: {
      ElTooltip: Tooltip,
      ElCheckbox: Checkbox,
      ElInputNumber: InputNumber,
    },
    props: {
      pan: {
        type: Number,
        required: true,
      },
      tilt: {
        type: Number,
        required: true,
      },
      value: {
        type: Object,
        required: true,
      },
      selected: {
        type: Boolean,
        default: false,
      },
    },
    data() {
      return {
        innerValue: this.value,
      };
    },
    watch: {
      value(v) {
        this.innerValue = { ...v };
      },
      pan(p) {
        if(this.selected) this.innerValue.pan = Math.round(p);
      },
      tilt(t) {
        if(this.selected) this.innerValue.tilt = Math.round(t);
      },
    },
    methods: {
      Pan() {
        if(this.selected) this.$emit('pan', this.innerValue.pan);
        this.Submit();
      },
      Tilt() {
        if(this.selected) this.$emit('tilt', this.innerValue.tilt);
        this.Submit();
      },
      Submit() {
        this.$emit('input', this.innerValue);
      },
    },
  };
</script>

<style scoped>
  .cruise {
    display: inline-flex;
    flex-direction: column;
    width: 95%;
  }
  .selected {
    background-color: #f1f6f0;
  }
  .list-button {
    padding: 0;
    text-alin:right;
    float:right;
  }
</style>
