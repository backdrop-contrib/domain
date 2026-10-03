<?php

/**
 * @file
 * Request-local configuration overrides for Domain Access.
 */

/**
 * Overrides a configuration value without writing to active storage.
 *
 * Keep globals in sync for non-static configuration objects, and refresh the
 * cached object for static configurations such as system.core. Do not change
 * Config::$data: administrative saves must still save the underlying values.
 */
function domain_runtime_config_set($name, $key, $value) {
  $parts = explode('.', $key);
  $key = array_shift($parts);
  $original = &backdrop_static('domain_runtime_config_original', array());
  if (!isset($original[$name]) || !array_key_exists($key, $original[$name])) {
    $overrides = isset($GLOBALS['config'][$name]) ? $GLOBALS['config'][$name] : array();
    $original[$name][$key] = array(
      'exists' => array_key_exists($key, $overrides),
      'value' => array_key_exists($key, $overrides) ? $overrides[$key] : NULL,
    );
  }
  if ($parts) {
    $nested = config($name)->get($key);
    if (!is_array($nested)) {
      $nested = array();
    }
    $target = &$nested;
    foreach ($parts as $part) {
      if (!isset($target[$part]) || !is_array($target[$part])) {
        $target[$part] = array();
      }
      $target = &$target[$part];
    }
    $target = $value;
    $value = $nested;
  }
  $GLOBALS['config'][$name][$key] = $value;
  config($name)->setOverrides($GLOBALS['config'][$name]);
}

/**
 * Removes previous domain overrides when bootstrapping another domain.
 *
 * Restore pre-existing settings.php overrides and retain unrelated overrides.
 */
function domain_runtime_config_reset() {
  $original = &backdrop_static('domain_runtime_config_original', array());
  foreach ($original as $name => $keys) {
    foreach ($keys as $key => $previous) {
      if ($previous['exists']) {
        $GLOBALS['config'][$name][$key] = $previous['value'];
      }
      else {
        unset($GLOBALS['config'][$name][$key]);
      }
    }
    config($name)->setOverrides($GLOBALS['config'][$name]);
  }
  $original = array();
}
