/*
 * Lugaru legacy-GL emulation fixes.
 *
 * Included with: --js-library gl_legacy_patch.js
 *
 * Emscripten's legacy GL emulation (libglemu.js) does not fully implement
 * desktop OpenGL fixed-function state.  These patches fix four problems that
 * affect Lugaru's rendering:
 *
 *   1. FFP shaders must always include/use the texture matrix.  Otherwise an
 *      early cached renderer can be generated before glMatrixMode(GL_TEXTURE)
 *      is called and that renderer permanently ignores texture transforms.
 *   2. A new renderer's textureMatrixVersion must start at -1 so the first
 *      draw uploads u_textureMatrix instead of leaving the uniform at zero.
 *   3. GL_COLOR_MATERIAL must be tracked.  When enabled, glColor* supplies
 *      the ambient/diffuse material (and alpha), which is required for the
 *      tutorial ghost (alpha 0.6) and for correctly lit cliffs/characters.
 *   4. Generated fragment shaders should prefer highp when available to avoid
 *      texture-coordinate banding on mobile/Apple GPUs.
 */

mergeInto(LibraryManager.library, {
  $LugaruGLPatch__force: true,
  $LugaruGLPatch__deps: ['$GLImmediate', '$GLEmulation'],
  $LugaruGLPatch__postset: 'LugaruGLPatch.install();',

  $LugaruGLPatch: {
    installed: false,
    enableHooksInstalled: false,
    shaderSourceContext: null,

    install: function () {
      if (this.installed) {
        return;
      }
      this.installed = true;

      if (typeof GLImmediate === 'undefined' || typeof GLEmulation === 'undefined') {
        console.error('[Lugaru] GL patch: legacy GL emulation is not available.');
        return;
      }

      // (1) Always generate texture-matrix-aware FFP shaders, even for
      // renderers cached before the game first calls glMatrixMode(GL_TEXTURE).
      GLImmediate.useTextureMatrix = true;

      // (3) GL_COLOR_MATERIAL defaults, matching desktop OpenGL:
      // face = GL_FRONT_AND_BACK, mode = GL_AMBIENT_AND_DIFFUSE.
      GLEmulation.colorMaterialEnabled = false;
      GLEmulation.colorMaterialFace = 0x0408;
      GLEmulation.colorMaterialMode = 0x1602;

      // (2) Renderer.init() creates this array filled with zeros.  Zero is
      // also the initial value in GLImmediate.matrixVersion, so the first
      // upload would be skipped and u_textureMatrix would stay all zeros.
      var origCreateRenderer = GLImmediate.createRenderer;
      GLImmediate.createRenderer = function () {
        var renderer = origCreateRenderer.apply(this, arguments);
        if (renderer && renderer.textureMatrixVersion) {
          for (var i = 0; i < renderer.textureMatrixVersion.length; i++) {
            renderer.textureMatrixVersion[i] = -1;
          }
        }
        return renderer;
      };

      // (3) Install the color-material state hooks after GLEmulation.init()
      // has installed its own glEnable/glDisable wrappers.  This also works
      // if the library postset runs before that initialization.
      var origGLEmulationInit = GLEmulation.init;
      GLEmulation.init = function () {
        var result = origGLEmulationInit.apply(this, arguments);
        LugaruGLPatch.installEnableHooks();
        return result;
      };
      if (GLEmulation.hasRunInit) {
        this.installEnableHooks();
      }

      // (3+4) The renderer cache and shader source can only be patched once
      // the WebGL context exists, which happens in GLImmediate.init().
      var origImmediateInit = GLImmediate.init;
      GLImmediate.init = function () {
        var result = origImmediateInit.apply(this, arguments);
        LugaruGLPatch.installContextHooks();
        return result;
      };
      if (GLImmediate.rendererCache) {
        this.installContextHooks();
      }

      console.log('[Lugaru] Legacy GL emulation patch installed.');
    },

    installEnableHooks: function () {
      if (this.enableHooksInstalled) {
        return;
      }
      this.enableHooksInstalled = true;

      var prevEnable = _glEnable;
      _glEnable = _emscripten_glEnable = function (cap) {
        if (cap === 0x0B57 /* GL_COLOR_MATERIAL */) {
          if (!GLEmulation.colorMaterialEnabled) {
            GLEmulation.colorMaterialEnabled = true;
            if (GLImmediate.lastRenderer) {
              GLImmediate.lastRenderer.cleanup();
            }
            GLImmediate.currentRenderer = null;
          }
          return;
        }
        return prevEnable(cap);
      };

      var prevDisable = _glDisable;
      _glDisable = _emscripten_glDisable = function (cap) {
        if (cap === 0x0B57 /* GL_COLOR_MATERIAL */) {
          if (GLEmulation.colorMaterialEnabled) {
            GLEmulation.colorMaterialEnabled = false;
            if (GLImmediate.lastRenderer) {
              GLImmediate.lastRenderer.cleanup();
            }
            GLImmediate.currentRenderer = null;
          }
          return;
        }
        return prevDisable(cap);
      };

      var prevIsEnabled = _glIsEnabled;
      _glIsEnabled = _emscripten_glIsEnabled = function (cap) {
        if (cap === 0x0B57 /* GL_COLOR_MATERIAL */) {
          return GLEmulation.colorMaterialEnabled ? 1 : 0;
        }
        return prevIsEnabled(cap);
      };
    },

    installContextHooks: function () {
      if (typeof GLctx === 'undefined' || !GLctx) {
        return;
      }
      if (this.shaderSourceContext === GLctx) {
        return;
      }
      this.shaderSourceContext = GLctx;

      // (3) Include GL_COLOR_MATERIAL state in the FFP renderer cache key.
      // getRenderer() resets the shared key view and then appends its own
      // state.  Injecting the color-material bit at reset() gives the cache
      // a dedicated dimension for it without duplicating getRenderer().
      var cache = GLImmediate.rendererCache;
      if (cache && !cache.__lugaruColorMaterialKey) {
        cache.__lugaruColorMaterialKey = true;
        var origGetStaticKeyView = cache.getStaticKeyView;
        cache.getStaticKeyView = function () {
          var view = origGetStaticKeyView.apply(this, arguments);
          if (!view.__lugaruColorMaterialKey) {
            view.__lugaruColorMaterialKey = true;
            var origReset = view.reset;
            view.reset = function () {
              var result = origReset.apply(this, arguments);
              if (GLEmulation.colorMaterialEnabled) {
                this.next(1);
                this.next(GLEmulation.colorMaterialMode | 0);
              } else {
                this.next(0);
                this.next(0);
              }
              return result;
            };
          }
          return view;
        };
      }

      // (3+4) Rewrite generated FFP shader source at upload time.
      if (this.shaderSourceContext.__lugaruShaderSourcePatched) {
        return;
      }
      this.shaderSourceContext.__lugaruShaderSourcePatched = true;

      var origShaderSource = GLctx.shaderSource;
      GLctx.shaderSource = function (shader, source) {
        var shaderType = 0;
        try {
          shaderType = GLctx.getShaderParameter(shader, GLctx.SHADER_TYPE);
        } catch (e) {
          // Some implementations may not allow querying before source upload.
        }
        var patched = source;

        // (4) Prefer highp in fragment shaders, falling back to mediump on
        // GPUs that do not advertise fragment highp support.
        if (shaderType === GLctx.FRAGMENT_SHADER &&
            patched.indexOf('precision highp float;') < 0 &&
            patched.indexOf('precision mediump float;') >= 0) {
          patched = patched.replace(
            'precision mediump float;',
            '#ifdef GL_FRAGMENT_PRECISION_HIGH\nprecision highp float;\n#else\nprecision mediump float;\n#endif'
          );
        }

        // (3) With GL_COLOR_MATERIAL enabled, the current glColor becomes the
        // material ambient/diffuse.  FFP lighting uses these uniforms, so
        // substitute a_color (and its alpha) in the generated shader.
        if (GLEmulation.colorMaterialEnabled &&
            patched.indexOf('uniform vec4 u_materialAmbient;') >= 0) {
          var mode = GLEmulation.colorMaterialMode;
          if (mode === 0x1200 /* GL_AMBIENT */ || mode === 0x1602 /* GL_AMBIENT_AND_DIFFUSE */) {
            patched = patched.replace(/u_materialAmbient\.xyz/g, 'a_color.xyz');
          }
          if (mode === 0x1201 /* GL_DIFFUSE */ || mode === 0x1602) {
            patched = patched.replace(/u_materialDiffuse\.xyz/g, 'a_color.xyz');
            patched = patched.replace(/v_color\.w = u_materialDiffuse\.w;/g, 'v_color.w = a_color.a;');
          }
        }

        return origShaderSource.call(this, shader, patched);
      };
    }
  },

  // GL_COLOR_MATERIAL can be configured with glColorMaterial().  Lugaru uses
  // the default (ambient+diffuse), but implement it so the state is complete.
  glColorMaterial: function (face, mode) {
    GLEmulation.colorMaterialFace = face;
    GLEmulation.colorMaterialMode = mode;
    GLImmediate.currentRenderer = null;
  },
  glColorMaterial__deps: ['$GLEmulation', '$GLImmediate']
});
