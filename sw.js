
const CACHE_NAME = 'rameshwar-cache-v2';

// ઓફલાઇન સેવ કરવા માટેની ફાઇલો
const ASSETS_TO_CACHE = [
  './',
  './index.html',
  './manifest.json',
  './icon-192.png',
  './icon-512.png'
];

// ઇન્સ્ટોલ થાય ત્યારે તમામ જરૂરી ફાઇલો કેશ કરવી
self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE_NAME).then((cache) => {
      return cache.addAll(ASSETS_TO_CACHE);
    })
  );
  self.skipWaiting();
});

// જૂની કેશ ડિલીટ કરવી
self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then((keys) => {
      return Promise.all(
        keys.map((key) => {
          if (key !== CACHE_NAME) {
            return caches.delete(key);
          }
        })
      );
    })
  );
  event.waitUntil(clients.claim());
});

// ફ્રન્ટએન્ડ માટે ફાસ્ટ નેટવર્ક અને ઓફલાઇન સપોર્ટ
self.addEventListener('fetch', (event) => {
  // ESP32 કમાન્ડ કે સ્ટેટસ API હોય તેને કેશ ન કરવું (સીધું નેટવર્ક જ વાપરવું)
  if (event.request.url.includes('/cmd') || event.request.url.includes('/status') || event.request.url.includes('/god_icon')) {
    return;
  }

  event.respondWith(
    fetch(event.request)
      .then((networkResponse) => {
        return networkResponse;
      })
      .catch(() => {
        // નેટ બંધ હોય ત્યારે કેશમાંથી ફાઇલ બતાવવી
        return caches.match(event.request).then((cachedResponse) => {
          if (cachedResponse) {
            return cachedResponse;
          }
          if (event.request.mode === 'navigate') {
            return caches.match('./index.html');
          }
        });
      })
  );
});
