
const CACHE_NAME = 'rameshwar-cache-v1';

// ઇન્સ્ટોલ થાય ત્યારે
self.addEventListener('install', (event) => {
  self.skipWaiting();
});

// એક્ટિવેટ થાય ત્યારે
self.addEventListener('activate', (event) => {
  event.waitUntil(clients.claim());
});

// જ્યારે રિમોટ કમાન્ડ મોકલે ત્યારે નેટવર્કનો જ ઉપયોગ કરે (જેથી જૂનો ડેટા ન પકડી રાખે)
self.addEventListener('fetch', (event) => {
  event.respondWith(
    fetch(event.request).catch(() => {
      return caches.match(event.request);
    })
  );
});