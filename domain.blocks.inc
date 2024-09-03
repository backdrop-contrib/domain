<?php

/**
 * @file
 * Block view functions for Domain Access.
 */

/**
 * A nifty little domain-switcher block, useful during debugging.
 *
 * @see domain_block_view()
 */
function domain_block_view_switcher() {
  $output = '';
  $list = [];
  $domains = domain_domains();
  $msg = FALSE;
  foreach ($domains as $domain) {
    if ($domain['valid']) {
      $title = $domain['sitename'];
      $allow = TRUE;
    }
    else {
      $title = $domain['sitename'] . ' *';
      $allow = FALSE;
      if (user_access('access inactive domains')) {
        $msg = TRUE;
        $allow = TRUE;
      }
    }
    if ($allow) {
      $list[] = l($title, domain_get_uri($domain), ['absolute' => TRUE]);
    }
  }
  $items = ['items' => $list];
  $output = theme('item_list', $items);
  $block = [
    'subject' => t('Domain switcher'),
    'content' => [
      '#markup' => $output,
    ],
  ];
  return $block;
}

/**
 * Prints information about the current node.
 *
 * @see domain_block_view()
 */
function domain_block_view_information() {
  $output = '';
  $node = menu_get_object();
  if (empty($node->nid)) {
    return;
  }
  // Print the assigned domains.
  if (!empty($node->subdomains)) {
    $output .= theme('item_list', [
      'items' => $node->subdomains,
      'title' => t('Assigned domains'),
    ]);
  }
  // Print the link source domain.
  $this_domain = domain_get_node_match($node->nid);
  $output .= theme('item_list', [
    'items' => [check_plain($this_domain['sitename'])],
    'title' => t('Source domain'),
  ]);
  if (empty($output)) {
    $output = t('This node is not assigned to a domain.');
  }
  else {
    $output = '<p>' . t('%node is published with the following Domain Access rules:', ['%node' => $node->title]) . '</p>' . $output;
  }

  $block = [
    'subject' => t('Domain access information'),
    'content' => [
      '#markup' => $output,
    ],
  ];
  return $block;
}

/**
 * Prints information about the current HTTP request.
 *
 * @see domain_block_view()
 */
function domain_block_view_server() {
  $_domain = domain_get_domain();
  $output = '';
  $header = [t('Property'), t('Value')];
  $rows = [];
  $rows[] = [
    t('HTTP_HOST request'),
    check_plain($_SERVER['HTTP_HOST']),
  ];

  $check = domain_lookup(NULL, $_SERVER['HTTP_HOST']);
  $match = t('TRUE');
  if ($check == -1) {
    // Specific check for Domain Alias.
    if (isset($_domain['active_alias_id'])) {
      $match = t('ALIAS: Using alias %id', ['%id' => $_domain['active_alias_id']]);
    }
    else {
      $match = t('FALSE: Using default domain.');
    }
  }
  $rows[] = [
    t('Domain match'),
    $match,
  ];
  foreach ($_domain as $key => $value) {
    if (is_null($value)) {
      $value = t('NULL');
    }
    elseif ($value === TRUE) {
      $value = t('TRUE');
    }
    elseif ($value === FALSE) {
      $value = t('FALSE');
    }
    $rows[] = [
      check_plain($key),
      !is_array($value) ? check_plain($value) : _domain_block_print_array($value),
    ];
  }
  $output = theme('table', ['header' => $header, 'rows' => $rows]);
  $block = [
    'subject' => t('Domain server information'),
    'content' => [
      '#markup' => $output,
    ],
  ];
  return $block;
}

/**
 * Prints array data for the server block.
 *
 * @param $array
 *  An array of data. Note that we support two levels of nesting.
 *
 * @return
 *  A suitable output string.
 */
function _domain_block_print_array($array) {
  $items = [];
  foreach ($array as $key => $val) {
    $value = 'array';
    if (!is_array($val)) {
      $value = check_plain($val);
    }
    else {
      $list = [];
      foreach ($val as $k => $v) {
        $list[] = t('@key : @value', ['@key' => $k, '@value' => $v]);
      }
      $value = implode('<br />', $list);
    }
    $items[] = t('@key : !value', ['@key' => $key, '!value' => $value]);
  }
  return theme('item_list', ['items' => $items]);
}
