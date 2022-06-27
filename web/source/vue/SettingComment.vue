<template>
  <ElRow>
    <ElCol :offset="titleOffset" :span="9 - titleOffset">
      <h4>{{ title }}</h4>
    </ElCol>
    <ElCol :span="span">
      <span class="comment">
        <slot />
      </span>
    </ElCol>
  </ElRow>
</template>

<script>
  export default {
    props: {
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
        default: 15,
      },
    },
  };
</script>

<style scoped>
.comment {
  font-size: 1em;
}
</style>
