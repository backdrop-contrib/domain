<?php

/**
 * @file
 * Domain hooks for Domain Conf.
 *
 * @ingroup domain_conf
 */

/**
 * Implements hook_domain_warning().
 */
function domain_conf_domain_warning() {
  // These are the forms for variables set by Domain Conf.
  $forms = [
    'system_admin_theme_settings',
    'system_performance_settings',
    'system_regional_settings',
    'system_site_information_settings',
    'locale_languages_overview_form',
    'menu_configure',
    'node_type_form',
  ];
  $return = [];
  foreach ($forms as $form) {
    $return[$form] = 'admin/structure/domain/view/%domain_id/config';
  }
  return $return;
}

/**
 * Implements hook_domain_batch().
 */
function domain_conf_domain_batch() {
  $batch = [];
  // Allows the deletion of all Domain Configuration rows.
  $batch['domain_conf'] = [
    '#form' => [
      '#title' => t('Reset configurations'),
      '#type' => 'checkbox',
      '#options' => [0 => 1, 1 => t('Reset')],
      '#description' => t('Delete custom settings for this domain.'),
    ],
    '#permission' => 'administer site configuration',
    '#domain_action' => 'domain_delete',
    '#system_default' => 0,
    '#variable' => 'domain_conf',
    '#meta_description' => t('Delete custom settings for domains as supplied by Domain Configuration.'),
    '#table' => 'domain_conf',
    '#weight' => -10,
  ];
  // Change the site name.
  $batch['site_name'] = [
    '#form' => [
      '#title' => t('Site name'),
      '#type' => 'textfield',
      '#size' => 60,
      '#maxlength' => 255,
      '#description' => t('The site name for this domain. Usually overridden by the domain name.'),
    ],
    '#permission' => 'administer site configuration',
    '#domain_action' => 'domain_conf',
    '#system_default' => config_get('system.core', 'site_name'),
    '#variable' => 'site_name',
    '#meta_description' => t('Set the site name for all domains.'),
    '#data_type' => 'string',
    '#weight' => -8,
    '#update_all' => TRUE,
    '#group' => t('Site configuration'),
  ];
  // Change the email address.
  $batch['site_mail'] = [
    '#form' => [
      '#title' => t('Email address'),
      '#type' => 'textfield',
      '#size' => 40,
      '#maxlength' => 255,
      '#description' => t('Set the email address for this domain.'),
    ],
    '#permission' => 'administer site configuration',
    '#domain_action' => 'domain_conf',
    '#system_default' => config_get('system.core', 'site_name'),
    '#variable' => 'site_mail',
    '#meta_description' => t('Set the email address for all domains.'),
    '#data_type' => 'string',
    '#weight' => -8,
    '#update_all' => TRUE,
    '#group' => t('Site configuration'),
  ];
  // Change the site slogan.
  $batch['site_slogan'] = [
    '#form' => [
      '#title' => t('Site slogan'),
      '#type' => 'textfield',
      '#size' => 60,
      '#maxlength' => 255,
      '#description' => t('The slogan of this domain. Some themes display a slogan when available.'),
    ],
    '#permission' => 'administer site configuration',
    '#domain_action' => 'domain_conf',
    '#system_default' => config_get('system.core', 'site_slogan'),
    '#variable' => 'site_slogan',
    '#meta_description' => t('Set the site slogan for all domains.'),
    '#data_type' => 'string',
    '#weight' => -8,
    '#update_all' => TRUE,
    '#group' => t('Site configuration'),
  ];
  // Change the site front page.
  $batch['site_frontpage'] = [
    '#form' => [
      '#title' => t('Site frontpage'),
      '#type' => 'textfield',
      '#size' => 30,
      '#maxlength' => 255,
      '#description' => t('The home page displays content from this relative URL. If unsure, specify "node".'),
    ],
    '#permission' => 'administer site configuration',
    '#domain_action' => 'domain_conf',
    '#system_default' => config_get('system.core', 'site_frontpage'),
    '#variable' => 'site_frontpage',
    '#meta_description' => t('Set the site frontpage for all domains.'),
    '#data_type' => 'string',
    '#weight' => -8,
    '#update_all' => TRUE,
    '#group' => t('Site configuration'),
  ];
  // Change the anonymous user name.
  $batch['anonymous'] = [
    '#form' => [
      '#title' => t('Anonymous user'),
      '#type' => 'textfield',
      '#size' => 30,
      '#maxlength' => 255,
      '#description' => t('The name used to indicate anonymous users for this domain.'),
    ],
    '#permission' => 'administer site configuration',
    '#domain_action' => 'domain_conf',
    '#system_default' => config_get('system.core', 'anonymous'),
    '#variable' => 'anonymous',
    '#meta_description' => t('Set the anonymous user label for all domains.'),
    '#data_type' => 'string',
    '#weight' => -8,
    '#update_all' => TRUE,
    '#group' => t('Site configuration'),
  ];
  // Change the administrative theme.
  $themes = list_themes();
  ksort($themes);
  $options[] = t('Use domain default theme');
  foreach ($themes as $key => $value) {
    $options[$key] = $key;
  }
  $batch['admin_theme'] = [
    '#form' => [
      '#title' => t('Administrative theme'),
      '#type' => 'select',
      '#options' => $options,
      '#description' => t('Select the administrative theme for this domain.'),
    ],
    '#permission' => 'administer themes',
    '#domain_action' => 'domain_conf',
    '#system_default' => config_get('system.core', 'admin_theme'),
    '#variable' => 'admin_theme',
    '#meta_description' => t('Set the administrative theme for all domains.'),
    '#data_type' => 'string',
    '#weight' => -8,
    '#update_all' => TRUE,
    '#group' => t('Administrative theme'),
  ];
  // Choose whether the admin theme is used when creating/editing content
  $batch['node_admin_theme'] = [
    '#form' => [
      '#type' => 'checkbox',
      '#title' => t('Use the administration theme when editing or creating content'),
      '#description' => t('Choose whether to use the administrative theme on node add and edit pages on this domain.'),
    ],
    '#permission' => 'administer themes',
    '#domain_action' => 'domain_conf',
    '#system_default' => config_get('system.core', 'node_admin_theme'),
    '#variable' => 'node_admin_theme',
    '#meta_description' => t('Choose whether to use the administrative theme when editing or creating content on all domains.'),
    '#data_type' => 'boolean',
    '#weight' => -7,
    '#update_all' => TRUE,
    '#group' => t('Administrative theme'),
  ];

  // Change the timezone.
  $zones = system_time_zones();
  $batch['date_default_timezone'] = [
    '#form' => [
      '#title' => t('Timezone default'),
      '#type' => 'select',
      '#options' => $zones,
      '#description' => t('Select the default site time zone.'),
    ],
    '#permission' => 'administer site configuration',
    '#domain_action' => 'domain_conf',
    '#system_default' => date_default_timezone_get(),
    '#variable' => 'date_default_timezone',
    '#meta_description' => t('Set the default timezone for all domains.'),
    '#data_type' => 'string',
    '#weight' => -6,
    '#update_all' => TRUE,
    '#group' => t('Timezone settings'),
  ];
  // Change the caching mode.
  $cache = config_get('system.core', 'cache');
  $batch['cache'] = [
    '#form' => [
      '#type' => 'checkbox',
      '#title' => t('Cache pages for anonymous users'),
      '#default_value' => $cache,
    ],
    '#permission' => 'administer site configuration',
    '#domain_action' => 'domain_conf',
    '#system_default' => $cache,
    '#variable' => 'cache',
    '#meta_description' => t('Set the page cache options for all domains.'),
    '#data_type' => 'integer',
    '#weight' => -5,
    '#update_all' => TRUE,
    '#group' => t('Performance'),
  ];
  // Change the cache lifetime.
  $period = backdrop_map_assoc([
    0,
    60,
    180,
    300,
    600,
    900,
    1800,
    2700,
    3600,
    10800,
    21600,
    32400,
    43200,
    86400,
  ], 'format_interval');
  $period[0] = '<' . t('none') . '>';
  $batch['cache_lifetime'] = [
    '#form' => [
      '#type' => 'select',
      '#title' => t('Minimum cache lifetime'),
      '#default_value' => config_get('system.core', 'page_cache_maximum_age'),
      '#options' => $period,
      '#description' => t('The minimum amount of time that will elapse before the caches are recreated.'),
    ],
    '#permission' => 'administer site configuration',
    '#domain_action' => 'domain_conf',
    '#system_default' => config_get('system.core', 'page_cache_maximum_age'),
    '#variable' => 'cache_lifetime',
    '#meta_description' => t('Set the minimum cache lifetime for all domains.'),
    '#data_type' => 'integer',
    '#weight' => -5,
    '#update_all' => TRUE,
    '#group' => t('Performance'),
  ];
  // Change the default language.
  if (module_exists('locale')) {
    $languages = domain_conf_language_options();
    $default = language_default();
    $batch['language_default'] = [
      '#form' => [
        '#title' => t('Default language'),
        '#type' => 'select',
        '#options' => $languages,
        '#description' => t('The default language to use for this domain. <em>Note: This setting only works with path-based language switching.</em>'),
      ],
      '#permission' => 'administer languages',
      '#domain_action' => 'domain_conf',
      '#system_default' => $default->langcode,
      '#override_default' => TRUE,
      '#variable' => '',
      // deliberately empty, since this is an array and cannot be set.
      '#meta_description' => t('Set the default language for all domains.'),
      '#data_type' => 'string',
      '#weight' => 6,
      '#update_all' => TRUE,
      '#group' => t('Language settings'),
      '#module' => t('Language'),
    ];
  }
  foreach ($batch as $key => $value) {
    if (!isset($batch[$key]['#module'])) {
      $batch[$key]['#module'] = t('Domain Configuration');
    }
  }

  return $batch;
}

/**
 * Implements hook_domain_delete().
 */
function domain_conf_domain_delete($domain, $form_values = []) {
  domain_conf_data_delete($domain['domain_id']);
  cache_clear_all('variables', 'cache_bootstrap');
}

/**
 * Form process function to load available menus.
 *
 * If we try to load the menus in the batch hook, it can create a race
 * condition with menu_rebuild() that leads to a fatal call stack error.
 *
 * So we use this process callback to populate the menus safely, after they
 * have been rebuilt.
 */
function domain_conf_get_menu_parents($element, &$form_state) {
  static $options;
  if (!isset($options)) {
    $options = menu_parent_options(menu_get_menus(), ['mlid' => 0]);
  }
  $element['#options'] += $options;
  return $element;
}
