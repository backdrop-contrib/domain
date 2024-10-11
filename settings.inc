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

  $database_is_set = FALSE;
  // Database connection settings have been provided in the form of a connection
  // string.
  if (!empty($database) && is_string($database)) {
    $default_db_string = 'mysql://user:pass@localhost/database_name';
    if ($database !== $default_db_string) {
      // Convert simplified database settings string into the full databases
      // array.
      $database_parts = parse_url($database);
      if (!$database_parts) {
        trigger_error('The database setting could not be parsed. Please check the $database setting in settings.php.', E_USER_ERROR);
      }
      $databases['default']['default'] = array(
        'driver' => $database_parts['scheme'],
        // Remove leading slash.
        'database' => rawurldecode(substr($database_parts['path'], 1)),
        'username' => isset($database_parts['user']) ? rawurldecode($database_parts['user']) : '',
        'password' => isset($database_parts['pass']) ? rawurldecode($database_parts['pass']) : '',
        'host' => $database_parts['host'],
        'port' => isset($database_parts['port']) ? $database_parts['port'] : NULL,
        'prefix' => !empty($database_prefix) ? $database_prefix : '',
      );
      $database_is_set = TRUE;
    }
  }
  // Database connection settings have been provided in the form of a simplified
  // array.
  if (!empty($database) && is_array($database)) {
    $default_db_array = array(
      'database' => 'database_name',
      'username' => 'user',
      'password' => 'pass',
      'host' => 'localhost',
    );
    if ($database != $default_db_array) {
      // Specify default values.
      $database += array(
        'prefix' => '',
        'driver' => 'mysql',
        'port' => NULL,
        'charset' => 'utf8mb4',
        'collation' => 'utf8mb4_general_ci',
      );
      $databases['default']['default'] = $database;
      $database_is_set = TRUE;
    }
  }
  if ($database_is_set) {
    // Check that all required parts are included.
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
