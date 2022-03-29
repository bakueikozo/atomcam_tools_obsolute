<template>
  <ElTooltip :tabindex="-1" placement="top" :content="tooltip" effect="light" :open-delay="500">
    <ElRow>
      <ElCol :offset="titleOffset" :span="9 - titleOffset">
        <h4>{{ title }}</h4>
      </ElCol>
      <ElCol :span="text?8:4">
        <ElSwitch :value="value" @input="$emit('input', $event)" @change="$emit('change', $event)" :active-value="onOff ? 'on' : true" :inactive-value="onOff ? 'off' : false" :active-color="text?'#13ce66':'#409eff'" :active-text="text?text[0]:''" :inactive-color="text?'#409eff':'#c0ccda'" :inactive-text="text?text[1]:''" />
      </ElCol>
      <ElCol v-if="comment!==''" :span="text?7:11">
        {{ comment }}
      </ElCol>
    </ElRow>
  </ElTooltip>
</template>

<script>
  import { Tooltip, Switch } from 'element-ui';
  import 'element-ui/lib/theme-chalk/tooltip.css';
  import 'element-ui/lib/theme-chalk/switch.css';

  export default {
    components: {
      ElTooltip: Tooltip,
      ElSwitch: Switch,
    },
    props: {
      tooltip: {
        type: String,
        default: '',
      },
      titleOffset: {
        type: Number,
        default: 1,
      },
      title: {
        type: String,
        required: true,
      },
      value: {
        type: [ String, Boolean ],
        required: true,
      },
      onOff: {
        type: Boolean,
        default: true,
      },
      text: {
        type: Array,
        default: null,
      },
      comment: {
        type: String,
        default: '',
      },
    },
  };
</script>
