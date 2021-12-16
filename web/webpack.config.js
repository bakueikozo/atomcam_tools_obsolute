const webpack = require('webpack');
const { VueLoaderPlugin } = require('vue-loader');
const ESLintPlugin = require('eslint-webpack-plugin');
const BundleAnalyzer = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;
const CompressionPlugin = require("compression-webpack-plugin");

module.exports = (env, argv) => {
  const config = {
    entry: {
      bundle: [
        './source/js/index.js',
        './source/index.html',
      ],
    },
    output: {
      path: __dirname + '/frontend',
      filename: '[name].js',
    },
    module: {
      rules: [
        {
          test: /index.html/,
          type: 'asset/resource',
          generator: {
            filename: '[name][ext]',
          },
        },
        {
          test: /\.vue$/,
          loader: 'vue-loader',
        },
        {
          test: /\.css$/,
          use: [ 'style-loader', 'css-loader' ],
        },
        {
          test: /\.(ttf|otf|woff|woff2)(\?.+)?$/,
          type: 'asset/resource',
          generator: {
            filename: 'fonts/[name][ext]',
          },
        },
      ],
    },
    resolve: {
      alias: {
        'vue$': 'vue/dist/vue.common.js',
      },
    },
    plugins: [
      new ESLintPlugin({
        extensions: ['js', 'vue' ],
        baseConfig: {
          extends: [
            'eslint:recommended',
          ],
          env: {
              node: true,
              es6: true,
          },
          parserOptions: {
            ecmaVersion: 2020,
          },
          rules: {
            'comma-dangle': [ 'error', {
               arrays: 'always-multiline',
               objects: 'always-multiline',
               functions: 'always-multiline',
               imports: 'always-multiline',
               exports: 'always-multiline',
            }],
            'no-control-regex': 'off',
            eqeqeq: ['error', 'smart'],
            'space-before-function-paren': ['error', {
              anonymous: 'never',
              named: 'never',
              asyncArrow: 'always',
            }],
            'keyword-spacing': ['error', { overrides: {
              if: { after: false },
              for: { after: false },
              while: { after: false },
              catch: { after: false },
              switch: { after: false },
            }}],
            semi: ['error', 'always'],
          },
          overrides: [
            {
              files: ['source/js/*.js', 'source/vue/*.vue'],
              parserOptions: {
                sourceType: 'module',
                allowImportExportEverywhere: true,
              },
              env: {
                browser: true,
              },
              globals: {
                Socket: true,
                Common: true,
                NODE_ENV: true,
                RELEASE_VER: true,
                google: true,
              },
              rules: {
                'no-console': 'warn',
              },
            },
            {
              files: ['source/*/*.vue'],
              extends: [
                'plugin:vue/recommended',
              ],
              rules: {
                'vue/max-attributes-per-line': 'off',
                'vue/attributes-order': 'off',
                'vue/attribute-hyphenation': 'off',
                'vue/singleline-html-element-content-newline': 'off',
              },
            },
          ],
        },
      }),
      new VueLoaderPlugin(),
      new CompressionPlugin({
        filename: '[base].gz',
        algorithm: 'gzip',
        test: /\.js$/,
        threshold: 0,
        minRatio: 0.8,
        deleteOriginalAssets: true,
      }),
//      new BundleAnalyzer(),
    ],
  };

  if(argv.mode === 'development') config.devtool = 'inline-source-map';
  return config;
};
