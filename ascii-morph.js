// ascii-morph.js
// Multi-layer Perlin noise field driving a full-viewport ASCII character grid.
// Each cell independently interpolates between characters based on noise value.
// Usage: initAsciiMorph(containerElement, options)

(function () {

  // --- Permutation table for Perlin noise ---
  const P = new Uint8Array(512);
  (function seedPerm() {
    const base = [];
    for (let i = 0; i < 256; i++) base[i] = i;
    for (let i = 255; i > 0; i--) {
      const j = Math.floor(Math.random() * (i + 1));
      [base[i], base[j]] = [base[j], base[i]];
    }
    for (let i = 0; i < 512; i++) P[i] = base[i & 255];
  })();

  function fade(t) { return t * t * t * (t * (t * 6 - 15) + 10); }
  function lerp(a, b, t) { return a + t * (b - a); }
  function grad(hash, x, y) {
    const h = hash & 3;
    const u = h < 2 ? x : y;
    const v = h < 2 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
  }

  function perlin2(x, y) {
    const xi = Math.floor(x) & 255, yi = Math.floor(y) & 255;
    const xf = x - Math.floor(x), yf = y - Math.floor(y);
    const u = fade(xf), v = fade(yf);
    const aa = P[P[xi] + yi], ab = P[P[xi] + yi + 1];
    const ba = P[P[xi + 1] + yi], bb = P[P[xi + 1] + yi + 1];
    return lerp(
      lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u),
      lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u),
      v
    );
  }

  function fbm(x, y, octaves, lacunarity, gain) {
    let val = 0, amp = 0.5, freq = 1, max = 0;
    for (let i = 0; i < octaves; i++) {
      val += perlin2(x * freq, y * freq) * amp;
      max += amp;
      amp *= gain;
      freq *= lacunarity;
    }
    return val / max;
  }

  const RAMP = ' .\'`rtfjxnuvczmwqpdbkhao ^",:;~-_=+<>[]{}|()1iIlXYUJCLQ0OZ*#MW&8%B@$';

  function noiseToChar(n) {
    const t = (n + 1) / 2;
    const idx = Math.floor(t * (RAMP.length - 1));
    return RAMP[Math.max(0, Math.min(RAMP.length - 1, idx))];
  }

  function initAsciiMorph(container, options = {}) {
    const {
      opacity     = 0.12,
      speed       = 0.25,
      cellW       = 13,
      cellH       = 20,
      octaves     = 4,
      lacunarity  = 2.1,
      gain        = 0.5,
      scale       = 0.065,
      layer2Scale = 0.035,
      layer2Weight= 0.4,
    } = options;

    // Wrapper covers the viewport via pure CSS (inset:0) — no JS measurement
    // needed for sizing. This avoids relying on vw/vh or innerWidth/innerHeight
    // values that can be stale or wrong in some browser/embedded contexts.
    const wrap = document.createElement('div');
    wrap.setAttribute('aria-hidden', 'true');
    Object.assign(wrap.style, {
      position:       'fixed',
      inset:          '0',
      overflow:       'hidden',
      pointerEvents:  'none',
      zIndex:         '0',
      opacity:        String(opacity),
    });

    const pre = document.createElement('pre');
    Object.assign(pre.style, {
      position:       'absolute',
      top:            '0',
      left:           '0',
      margin:         '0',
      padding:        '0',
      fontFamily:     "'Courier New', Courier, monospace",
      fontSize:       '13px',
      lineHeight:     '1.54',
      color:          '#ffffff',
      whiteSpace:     'pre',
      userSelect:     'none',
      willChange:     'contents',
    });
    wrap.appendChild(pre);
    container.prepend(wrap);

    // Overfill generously using screen dimensions (always available, always
    // correct) rather than trying to measure the viewport precisely. Any
    // excess is simply clipped by overflow:hidden on the wrapper, so there is
    // no way to undershoot and leave a gap.
    let cols, rows, t = 0, rafId;
    let running = true;

    function computeGridSize() {
      const w = Math.max(screen.width, window.innerWidth || 0, document.documentElement.clientWidth || 0) * 2;
      const h = Math.max(screen.height, window.innerHeight || 0, document.documentElement.clientHeight || 0) * 2;
      cols = Math.ceil(w / cellW);
      rows = Math.ceil(h / cellH);
    }

    function render() {
      if (!running) return;
      t += speed;

      const lines = [];
      for (let row = 0; row < rows; row++) {
        let line = '';
        for (let col = 0; col < cols; col++) {
          const nx = col * scale + t * 0.6;
          const ny = row * scale + t * 0.45;
          const n1 = fbm(nx, ny, octaves, lacunarity, gain);
          const n2 = fbm(
            col * layer2Scale - t * 0.3,
            row * layer2Scale + t * 0.2,
            octaves - 1, lacunarity, gain
          );
          const combined = n1 * (1 - layer2Weight) + n2 * layer2Weight;
          line += noiseToChar(combined);
        }
        lines.push(line);
      }
      pre.textContent = lines.join('\n');
      rafId = requestAnimationFrame(render);
    }

    computeGridSize();
    window.addEventListener('resize', computeGridSize);

    const mq = window.matchMedia('(prefers-reduced-motion: reduce)');
    if (!mq.matches) {
      rafId = requestAnimationFrame(render);
    }
    mq.addEventListener('change', e => {
      if (e.matches) { cancelAnimationFrame(rafId); pre.textContent = ''; }
      else { rafId = requestAnimationFrame(render); }
    });

    return {
      destroy() {
        running = false;
        cancelAnimationFrame(rafId);
        window.removeEventListener('resize', computeGridSize);
        wrap.remove();
      }
    };
  }

  window.initAsciiMorph = initAsciiMorph;
})();