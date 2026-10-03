<?php

/**
 * Standalone regression check using the actual Backdrop configuration API.
 * Run: php tests/domain_config_runtime.test.php /path/to/core/includes/config.inc
 * This is not a substitute for an integration test on a Backdrop installation.
 */
if (empty($argv[1]) || !is_file($argv[1])) {
  fwrite(STDERR, "Pass the path to Backdrop core/includes/config.inc.\n");
  exit(1);
}
set_error_handler(function ($severity, $message, $file, $line) {
  throw new ErrorException($message, 0, $severity, $file, $line);
});
function &backdrop_static($name, $default = NULL) {
  static $values = array();
  if (!array_key_exists($name, $values)) {
    $values[$name] = $default;
  }
  return $values[$name];
}
function settings_get($name, $default = NULL) { return $default; }
function backdrop_valid_test_ua() { return FALSE; }
function module_exists($name) { return $name === 'locale'; }
function language_list() {
  return array('de' => (object) array('langcode' => 'de'), 'en' => (object) array('langcode' => 'en'));
}
function language_default() { return language_list()['de']; }
function backdrop_json_encode($value) { return json_encode($value); }
function backdrop_array_get_nested_value(array &$array, array $parents, &$exists = NULL) {
  $value = $array;
  foreach ($parents as $parent) {
    if (!is_array($value) || !array_key_exists($parent, $value)) { $exists = FALSE; return NULL; }
    $value = $value[$parent];
  }
  $exists = TRUE;
  return $value;
}
function check($condition, $message) {
  if (!$condition) { throw new Exception($message); }
  $GLOBALS['checks']++;
}
require $argv[1];
$tmp = sys_get_temp_dir() . '/domain-config-test-' . bin2hex(random_bytes(6));
mkdir($tmp . '/core/includes', 0700, TRUE);
file_put_contents($tmp . '/core/includes/common.inc', '<?php');
define('BACKDROP_ROOT', $tmp);
$config_directories = array('active' => $tmp);
$fixtures = array(
  'system.core' => array('_config_static' => TRUE, 'site_name' => 'Base', 'site_slogan' => 'Base slogan', 'language_default' => 'de', 'cache' => 1, 'node_admin_theme' => 1, 'page_cache_maximum_age' => 60, 'theme_default' => 'original'),
  'system.date' => array('_config_static' => TRUE, 'default_timezone' => 'Europe/Berlin'),
  'domain.settings' => array('_config_static' => TRUE, 'domain_sitename_override' => FALSE),
  'domain_settings.settings' => array('forms_settings' => array('example' => array('file_json' => array('example.settings'), 1 => array('enabled' => FALSE)))),
  'example.settings' => array('enabled' => TRUE, 'nested' => array('one' => 'disk', 'two' => 'keep')),
);
foreach ($fixtures as $name => $data) { file_put_contents($tmp . '/' . $name . '.json', json_encode($data)); }
$config['system.core'] = array('site_name' => 'Settings override', 'site_mail' => 'existing@example.test');
$config['example.settings'] = array('nested' => array('one' => 'settings', 'two' => 'keep'));
$language = language_default();
$checks = 0;
$hashes = array();
foreach (glob($tmp . '/*.json') as $path) { $hashes[$path] = hash_file('sha256', $path); }
$cached = config('system.core');
require dirname(__DIR__) . '/domain_conf/domain_conf.module';
$domain_data = &backdrop_static('domain_conf_data_get', array());
$domain_data[1] = array('site_name' => 'Domain one', 'site_slogan' => '', 'cache' => 0, 'node_admin_theme' => FALSE, 'cache_lifetime' => 0, 'language_default' => 'en', 'site_mail' => 'domain-conf-ignore', 'date_default_timezone' => 'UTC');
$domain_data[2] = array();
for ($i = 0; $i < 30; $i++) {
  domain_conf_domain_bootstrap_full(array('domain_id' => 1, 'sitename' => 'One'));
  check($cached->get('site_name') === 'Domain one', 'Static cached config refreshed');
  check(config_get('system.core', 'site_slogan') === '', 'Empty slogan retained');
  check(config_get('system.core', 'cache') === 0, 'Disabled cache retained');
  check(config_get('system.core', 'node_admin_theme') === FALSE, 'False retained');
  check(config_get('system.core', 'page_cache_maximum_age') === 0, 'Zero cache lifetime retained');
  check(config_get('system.core', 'site_mail') === 'existing@example.test', 'Ignore sentinel respected');
  check(config_get('system.date', 'default_timezone') === 'UTC', 'Timezone override');
  check(config_get('example.settings', 'enabled') === FALSE, 'Non-static config override');
  check($language->langcode === 'en', 'Domain language');
  domain_conf_domain_bootstrap_full(array('domain_id' => 2, 'sitename' => 'Two'));
  check(config_get('system.core', 'site_name') === 'Settings override', 'Settings override restored');
  check(config_get('system.core', 'cache') === 1, 'Base cache restored');
  check(config_get('example.settings', 'enabled') === TRUE, 'Non-static config reset');
  check($language->langcode === 'de', 'Original language restored');
}
domain_runtime_config_set('example.settings', 'nested.one', 'domain');
check(config_get('example.settings', 'nested.one') === 'domain', 'Nested override replaces root override');
check(config_get('example.settings', 'nested.two') === 'keep', 'Nested siblings retained');
domain_runtime_config_set('system.core', 'site_name', 'temporary');
$config['system.core']['unrelated'] = 'added later';
domain_runtime_config_reset();
check(config_get('example.settings', 'nested.one') === 'settings', 'Nested original restored');
check(config_get('system.core', 'unrelated') === 'added later', 'Unrelated override retained');
foreach ($hashes as $path => $hash) { check(hash_file('sha256', $path) === $hash, 'Runtime must not modify ' . basename($path)); }
// Administrative saving remains possible and does not save runtime values.
domain_runtime_config_set('system.core', 'site_name', 'Runtime only');
config_set('system.core', 'theme_default', 'saved theme');
$saved = json_decode(file_get_contents($tmp . '/system.core.json'), TRUE);
check($saved['site_name'] === 'Base', 'Runtime site name never persisted');
check($saved['theme_default'] === 'saved theme', 'Administrative save persists');
foreach (glob($tmp . '/*.json') as $path) { unlink($path); }
unlink($tmp . '/core/includes/common.inc');
rmdir($tmp . '/core/includes'); rmdir($tmp . '/core'); rmdir($tmp);
echo "PASS: $checks checks with Backdrop configuration API.\n";
