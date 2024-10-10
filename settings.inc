<?php
/**
 *
 * @file
 * settings.inc
 *
 * This file should be included at the bottom of your settings.php file:
 *
 * include DRUPAL_ROOT . '/sites/all/modules/domain/settings.inc';
 *
 * If you have installed the domain module into a different folder than
 * /sites/all/modules/domain please adjust the path appropriately.
 *
 * @ingroup domain
 */

/**
 * Include bootstrap file and start bootstrap phases.
 */
include 'domain.bootstrap.inc';

// If the module is not installed properly, or Drupal not yet installed, we
// can explode rather maginificently. Using a try/catch, we can at least fail
// silently.
try {
  global $databases, $settings;

  // Convert simplified database settings into the full databases array.
  if (!empty($database) && $database !== 'mysql://user:pass@localhost/database_name') {
    $database_parts = parse_url($database);
    if (!$database_parts) {
      trigger_error('The database setting could not be parsed. Please check the $database setting in settings.php.', E_USER_ERROR);
    }
    $databases['default']['default'] = array(
      'driver' => $database_parts['scheme'],
      'database' => rawurldecode(substr($database_parts['path'], 1)), // Remove leading slash.
      'username' => isset($database_parts['user']) ? rawurldecode($database_parts['user']) : '',
      'password' => isset($database_parts['pass']) ? rawurldecode($database_parts['pass']) : '',
      'host' => $database_parts['host'],
      'port' => isset($database_parts['port']) ? $database_parts['port'] : NULL,
      'prefix' => !empty($database_prefix) ? $database_prefix : '',
    );
    $required_parts = array('driver', 'database', 'username', 'host');
    foreach ($required_parts as $key) {
      if (!$databases['default']['default'][$key]) {
        $missing_field = ($key == 'database') ? 'name' : $key;
        trigger_error('Could not find a value for the database "' . $missing_field . '" setting. Please check the $database setting in settings.php.', E_USER_ERROR);
      }
    }
  }
  domain_bootstrap();
}
catch(Exception $e) {
  // No error logging is available yet, assume we are installing Drupal.
  // See https://www.drupal.org/node/1559486.
}
