(() => {
  const TAG = 'general_magic-js';

  Pebble.addEventListener('ready', () => {
    console.log(`${TAG}: ready`);
  });

  Pebble.addEventListener('appmessage', (event) => {
    console.log(`${TAG}: app message ${JSON.stringify(event.payload)}`);
  });
})();
