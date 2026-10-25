mergeInto(LibraryManager.library, {
    emscripten_glDrawBuffer: function(buffer) {
        // No-op for WebGL single framebuffer
    },
    emscripten_glReadBuffer: function(buffer) {
        // No-op for WebGL single framebuffer
    },
    glDrawBuffer: function(buffer) {
        // No-op
    },
    glReadBuffer: function(buffer) {
        // No-op
    }
});
