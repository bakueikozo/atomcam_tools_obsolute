<template>
  <ElTooltip :tabindex="-1" placement="top" :content="tooltip" effect="light" :open-delay="500">
    <ElRow>
      <ElCol :offset="titleOffset" :span="9 - titleOffset">
        <h4>{{ title }}</h4>
      </ElCol>
      <ElCol :span="span">
        <slot>
          <ElButton :type="type" size="mini" @click="$emit('click', $event)">
            {{ label }}
          </ElButton>
        </slot>
      </ElCol>
    </ElRow>
  </ElTooltip>
</template>

<script>
  import { Tooltip } from 'element-ui';
  import 'element-ui/lib/theme-chalk/tooltip.css';

  export default {
    components: {
      ElTooltip: Tooltip,
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
      span: {
        type: Number,
        default: 6,
      },
      type: {
        type: String,
        default: 'primary',
      },
      label: {
        type: String,
        default: null,
      },
    },
  };
</script>
