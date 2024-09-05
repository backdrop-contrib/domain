<?php

/**
 * @file
 * Administration pages for Domain Content.
 *
 * @ingroup domain_content
 */

// Ensure that the node admin page functions are loaded.
module_load_include('inc', 'node', 'node.admin');

/**
 * The domain content page of menu callbacks.
 *
 * @return
 *   A link group for each domain the user can access.
 */
function domain_content_page() {
  $build = [];
  $content = [];
  $content = system_admin_menu_block(menu_get_item('admin/domain/content'));
  // Print the list of options.
  if (!empty($content)) {
    $output = theme('admin_block_content', ['content' => $content]);
  }
  // There was an error or invalid request.
  else {
    $output = t('There are no valid domains configured.');
  }
  $build['content'] = [
    '#markup' => $output,
  ];
  return $build;
}

/**
 * List the available domains for this user.
 *
 * See http://drupal.org/node/367752 for a discussion of the
 * need for this function.
 */
function domain_content_list() {
  global $user;
  domain_user_set($user);

  $build = [];
  $show_all = user_access('bypass node access') || user_access('review content for all domains');

  if (!$show_all) {
    if (empty($user->domain_user)) {
      return backdrop_access_denied();
    }
    $user_domains = array_filter($user->domain_user);
  }
  // Table information
  $header = [
    ['data' => t('Id'), 'field' => 'domain_id'],
    ['data' => t('Site content'), 'field' => 'sitename'],
    ['data' => t('Content count')],
    ['data' => t('Unpublished')],
    ['data' => t('Site'), 'field' => 'subdomain'],
  ];
  // Set up the base query.
  $query = db_select('domain', 'd')
    ->fields('d', [
      'is_default',
      'domain_id',
      'sitename',
      'subdomain',
      'scheme',
      'valid',
    ]);
  // Filter by user access to assigned domains.
  if (!$show_all) {
    $query->condition('domain_id', $user_domains, 'IN');
  }
  // Add the tablesort.
  $query = $query
    ->extend('TableSort')
    ->orderBy('is_default', 'DESC')
    ->orderByHeader($header);
  $query = $query->extend('PagerDefault')
    ->limit(config_get('domain.settings', 'domain_list_size'));
  // Get the domains.
  $result = $query->execute();
  foreach ($result as $data) {
    $domains[] = (array) $data;
  }
  // Create the table.
  $rows = [];
  foreach ($domains as $domain) {
    $path = trim(domain_get_path($domain), '/');
    $rows[] = [
      $domain['domain_id'] . (($domain['is_default']) ? ' * ' : ''),
      l(t('@sitename content', ['@sitename' => $domain['sitename']]), 'admin/domain/content/' . $domain['domain_id']),
      number_format((int) db_query("SELECT COUNT(nid) FROM {domain_access} WHERE gid = :gid AND realm = :realm", [
        ':gid' => $domain['domain_id'],
        ':realm' => 'domain_id',
      ])->fetchField()),
      number_format((int) db_query("SELECT COUNT(n.nid) FROM {node} n INNER JOIN {domain_access} da ON n.nid = da.nid WHERE da.gid = :gid AND da.realm = :realm AND n.status = 0", [
        ':gid' => $domain['domain_id'],
        ':realm' => 'domain_id',
      ])->fetchField()),
      l(t('View @domain', ['@domain' => $domain['subdomain']]), $path),
    ];
  }

  $all = [
    '-',
    l(t('Content assigned to all affiliates'), 'admin/domain/content/all'),
    number_format((int) db_query("SELECT COUNT(nid) FROM {domain_access} WHERE gid = :gid AND realm = :realm", [
      ':gid' => 0,
      ':realm' => 'domain_site',
    ])->fetchField()),
    number_format((int) db_query("SELECT COUNT(n.nid) FROM {node} n INNER JOIN {domain_access} da ON n.nid = da.nid WHERE da.gid = :gid AND da.realm = :realm AND n.status = 0", [
      ':gid' => 0,
      ':realm' => 'domain_site',
    ])->fetchField()),
    '',
  ];
  array_unshift($rows, $all);
  if (!empty($rows)) {
    $output = '<p>' . t('The table below shows all the affiliates sites whose content you may edit. Click on the site name link to see all content assigned to that affiliate. Note that not all users may view unpublished content.') . '</p>';
    $build['header'] = [
      '#markup' => $output,
    ];
    $build['content'] = [
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#attributes' => ['id' => 'domain-list'],
    ];
    $build['pager']['#theme'] = 'pager';
  }
  else {
    $build['content'] = [
      '#markup' => t('You do not have editing access to any domains. Please contact your site administrator.'),
    ];
  }
  return $build;
}

/**
 * Content administration for a specific domain.
 *
 * This callback puts the user on the current domain and then
 * fetches the appropriate content for batch editing.
 *
 * @param $domain_id
 *   The unique identifier for the currently active domain.
 * @param $all_affiliates
 *   A boolean flag that indicates whether to grant the domain_site node access
 *   realm for this content view.
 *
 * @return
 *   A link group for each domain the user can access.
 */
function domain_content_view($domain_id = NULL, $all_affiliates = FALSE) {
  $build = [];
  $_domain = domain_get_domain();
  // Load the active domain, which is not necessarily the current domain.
  if (!is_null($domain_id) && $domain_id != $_domain['domain_id']) {
    domain_set_domain($domain_id);
    $_domain = domain_get_domain();
  }
  $output = '';
  // Override the $_domain global so we can see the appropriate content
  if (!is_null($domain_id)) {
    $_domain['site_grant'] = FALSE;
    backdrop_set_title(t('Content for @domain', ['@domain' => $_domain['sitename']]));
  }
  elseif ($all_affiliates) {
    $_domain['site_grant'] = TRUE;
    backdrop_set_title(t('Content for all affiliate sites'));
  }

  // KILLSWITCH CASE: returns an error
  else {
    backdrop_set_message(t('Invalid request'), 'error');
    $build['content'] = [
      '#markup' => t('<p>The specified domain does not exist.</p>'),
    ];
    return $build;
  }
  $output = backdrop_get_form('domain_content_admin');
  // Reset the active domain.
  domain_reset_domain();
  return $output;
}

/**
 * Content admin page callback.
 *
 * @return
 *   A themed HTML batch content editing form.
 */
function domain_content_admin($form, &$form_state) {
  $form['#attached']['css'] = [backdrop_get_path('module', 'domain_content') . '/domain_content.css'];

  if (isset($form_state['values']['operation']) && $form_state['values']['operation'] == 'delete') {
    $form = node_multiple_delete_confirm($form, $form_state, array_filter($form_state['values']['nodes']));
    // Set the proper path for form actions.
    $form['actions']['cancel']['#href'] = $form['actions']['cancel']['#options']['path'] = request_path();
    $form['#submit'][] = 'domain_content_delete_multiple_submit';
    return $form;
  }
  $form['filter'] = domain_content_filter_form();
  $form['#submit'][] = 'domain_content_filter_form_submit';

  // Privileged users can make global changes to Domain Access permissions.
  if (user_access('set domain access')) {
    domain_content_add_form_widget($form);
  }
  $form['admin'] = domain_content_form();

  $form['admin']['options']['submit']['#submit'][] = 'domain_content_process_nodes';
  // Filter the available operations based on user permissions.
  domain_content_filter_operations($form['admin']['options']['operation']['#options']);

  return $form;
}

/**
 * On delete confirmation, redirect properly.
 */
function domain_content_delete_multiple_submit($form, &$form_state) {
  $form_state['redirect'] = $form['actions']['cancel']['#href'];
}

/**
 * Rewrites node_admin_nodes() to use db_rewrite_sql().
 *
 * @return
 *   A form array according to the FormsAPI.
 */
function domain_content_form() {
  $_domain = domain_get_domain();
  $admin_access = user_access('administer nodes');

  // Build the 'Update options' form.
  $form['options'] = [
    '#type' => 'fieldset',
    '#title' => t('Update options'),
    '#attributes' => ['class' => ['container-inline']],
  ];
  $options = [];
  $list = domain_content_node_operations();
  foreach ($list as $key => $value) {
    $options[$key] = $value['label'];
  }
  // Get the actions the user may perform.
  if ($admin_access) {
    foreach (module_invoke('domain_content', 'node_actions') as $operation => $array) {
      $options[$operation] = $array['label'];
    }
  }

  // No options? No form access.
  if (empty($options)) {
    $form['options']['#access'] = FALSE;
  }
  $form['options']['operation'] = [
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => 'approve',
  ];
  $form['options']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Update'),
    '#validate' => ['domain_content_admin_nodes_validate'],
    '#submit' => ['domain_content_admin_nodes_submit'],
  ];


  // Enable language column if translation module is enabled
  // or if we have any node with language.
  $multilanguage = (module_exists('translation') || db_query("SELECT 1 FROM {node} WHERE langcode <> :langcode", [':langcode' => LANGUAGE_NONE])->fetchField());

  // Build the sortable table header.
  $header = [
    'title' => ['data' => t('Title'), 'field' => 'n.title'],
    'domains' => ['data' => t('Affiliates')],
    'type' => ['data' => t('Type'), 'field' => 'n.type'],
    'author' => t('Author'),
    'status' => ['data' => t('Status'), 'field' => 'n.status'],
    'changed' => [
      'data' => t('Updated'),
      'field' => 'n.changed',
      'sort' => 'desc',
    ],
  ];

  if ($multilanguage) {
    $header['language'] = ['data' => t('Language'), 'field' => 'n.language'];
  }
  $header['operations'] = ['data' => t('Operations')];

  $query = db_select('node', 'n')->extend('PagerDefault')->extend('TableSort');
  domain_content_build_filter_query($query);

  if (!user_access('bypass node access')) {
    // If the user can view unpublished nodes on assigned domains, then we can
    // continue here.
    if (user_access('view unpublished domain content')) {
      // Do nothing.
    }
    // If the user is able to view their own unpublished nodes, allow them
    // to see these in addition to published nodes.
    elseif (user_access('view own unpublished content')) {
      $subselect = db_select('node', 'node')
        ->fields('node', ['nid'])
        ->condition('node.uid', $GLOBALS['user']->uid)
        ->condition('node.status', 0);
      $query->condition(db_or()
        ->condition('n.status', 1)
        ->condition('n.nid', $subselect, 'IN')
      );
    }
    // If not, restrict the query to published nodes.
    else {
      $query->condition('n.status', 1);
    }
  }

  // Write the proper query.
  $arg = arg(3);
  // If 'all', only show content assigned to all affiliates.
  if ($arg == 'all') {
    domain_content_alter_node_query($query, TRUE, FALSE);
  }
  else {
    // Restrict the query to the active domain.
    domain_content_alter_node_query($query, FALSE);
  }

  $nids = $query
    ->fields('n', ['nid'])
    ->limit(50)
    ->orderByHeader($header)
    ->execute()
    ->fetchCol();
  $nodes = node_load_multiple($nids);

  // Prepare the list of nodes.
  $languages = language_list();
  $destination = backdrop_get_destination();
  $options = [];
  foreach ($nodes as $node) {
    $l_options = (!empty($node->language) && $node->language != LANGUAGE_NONE) ? ['language' => $languages[$node->language]] : [];
    $options[$node->nid] = [
      'title' => [
        'data' => [
          '#type' => 'link',
          '#title' => $node->title,
          '#href' => 'node/' . $node->nid,
          '#options' => $l_options,
          '#suffix' => ' ' . theme('mark', ['type' => node_mark($node->nid, $node->changed)]),
        ],
      ],
      'domains' => domain_content_view_domains($node),
      // set the assigned domains
      'type' => check_plain(node_type_get_name($node)),
      'author' => theme('username', ['account' => $node]),
      'status' => $node->status ? t('published') : t('not published'),
      'changed' => format_date($node->changed, 'short'),
    ];
    if ($multilanguage) {
      $options[$node->nid]['language'] = (empty($node->language) || $node->language == LANGUAGE_NONE) ? t('Language neutral') : t($languages[$node->language]->name);
    }
    // Build a list of all the accessible operations for the current node.
    $operations = [];
    if (node_access('update', $node)) {
      $operations['edit'] = [
        'title' => t('edit'),
        'href' => 'node/' . $node->nid . '/edit',
        'query' => $destination,
      ];
    }
    if (node_access('delete', $node)) {
      $operations['delete'] = [
        'title' => t('delete'),
        'href' => 'node/' . $node->nid . '/delete',
        'query' => $destination,
      ];
    }
    $options[$node->nid]['operations'] = [];
    if (count($operations) > 1) {
      // Render an unordered list of operations links.
      $options[$node->nid]['operations'] = [
        'data' => [
          '#theme' => 'links__node_operations',
          '#links' => $operations,
          '#attributes' => ['class' => ['links', 'inline']],
        ],
      ];
    }
    elseif (!empty($operations)) {
      // Render the first and only operation as a link.
      $link = reset($operations);
      $options[$node->nid]['operations'] = [
        'data' => [
          '#type' => 'link',
          '#title' => $link['title'],
          '#href' => $link['href'],
          '#options' => ['query' => $link['query']],
        ],
      ];
    }
  }
  // Only use a tableselect when the current user is able to perform any
  // operations.
  if (!empty($operations)) {
    $form['nodes'] = [
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $options,
      '#empty' => t('No content available.'),
    ];
  }
  // Otherwise, use a simple table.
  else {
    $form['nodes'] = [
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $options,
      '#empty' => t('No content available.'),
    ];
  }

  $form['pager'] = ['#markup' => theme('pager', ['tags' => NULL])];

  return $form;
}

/**
 * Format domains for this node.
 */
function domain_content_view_domains($node) {
  $node_domains = [];
  if (!empty($node->domains)) {
    foreach ($node->domains as $domain) {
      // Convert the domain ids to data so we can print them.
      $node_domains[] = domain_lookup($domain);
    }
  }
  // If we have multiple domains, print them.
  $items = [];
  if ($node->domain_site) {
    $items[-1] = t('All affiliates');
  }
  if (!empty($node_domains)) {
    foreach ($node_domains as $item) {
      $items[$item['domain_id']] = check_plain($item['sitename']);
    }
  }
  if (module_exists('domain_source')) {
    $source = NULL;
    $source = db_query("SELECT domain_id FROM {domain_source} WHERE nid = :nid", [':nid' => $node->nid])->fetchField();
    if (!empty($source) && isset($items[$source])) {
      $items[$source] .= '*';
    }
  }
  return theme('item_list', ['items' => $items]);
}

/**
 * Filters the node batch operations based on the user's permissions.
 *
 * @param &$operations
 *   The complete list of operations, passed by reference.
 *
 * @return
 *   No return value. Modify by reference.
 */
function domain_content_filter_operations(&$operations) {
  // Administer nodes can do anything.
  if (user_access('administer nodes')) {
    return;
  }
  // Check for the delete perm.
  if (!user_access('delete domain content')) {
    unset($operations['delete']);
  }
}

/**
 * Abstraction to allow query alters outside of node access.
 *
 * @param $query
 *   A dynamic node query.
 * @param $all_affiliates
 *   Boolean value indicating whether to grant the 'domain_site' grant.
 * @param $current_domain
 *   Boolean value indicating whether to grant the 'domain_id' grant.
 */
function domain_content_alter_node_query(QueryAlterableInterface $query, $all_affiliates = TRUE, $current_domain = TRUE) {
  $_domain = domain_get_domain();
  $domain_id = (int) $_domain['domain_id'];
  $tables = $query->getTables();
  foreach ($tables as $nalias => $tableinfo) {
    $table = $tableinfo['table'];
    if (!($table instanceof SelectQueryInterface) && $table == 'node') {
      $access_alias = $query->join('domain_access', 'da_admin', "da_admin.nid = {$nalias}.nid");
      $or = db_or();
      if ($all_affiliates) {
        $or->condition(db_and()
          ->condition("{$access_alias}.gid", 0)
          ->condition("{$access_alias}.realm", 'domain_site')
        );
      }
      if ($current_domain) {
        $or->condition(db_and()
          ->condition("{$access_alias}.gid", $domain_id)
          ->condition("{$access_alias}.realm", 'domain_id')
        );
        if (user_access('view unpublished domain content')) {
          $or->condition(db_and()
            ->condition("{$access_alias}.gid", $domain_id)
            ->condition("{$access_alias}.realm", 'domain_unpublished')
          );
        }
      }
      $query->condition($or);
      // Node module will not have run, so add the distinct.
      $query->distinct();
    }
  }
}

/**
 * Form submission handler for node_filter_form().
 *
 * @see node_admin_content()
 * @see node_admin_nodes()
 * @see node_admin_nodes_submit()
 * @see node_admin_nodes_validate()
 * @see node_filter_form()
 * @see node_multiple_delete_confirm()
 * @see node_multiple_delete_confirm_submit()
 */
function domain_content_filter_form_submit($form, &$form_state) {
  $filters = domain_content_filters();
  switch ($form_state['values']['op']) {
    case t('Filter'):
    case t('Refine'):
      // Apply every filter that has a choice selected other than 'any'.
      foreach ($filters as $filter => $options) {
        if (isset($form_state['values'][$filter]) && $form_state['values'][$filter] != '[any]') {
          // Flatten the options array to accommodate hierarchical/nested options.
          $flat_options = form_options_flatten($filters[$filter]['options']);
          // Only accept valid selections offered on the dropdown, block bad input.
          if (isset($flat_options[$form_state['values'][$filter]])) {
            $_SESSION['node_overview_filter'][] = [
              $filter,
              $form_state['values'][$filter],
            ];
          }
        }
      }
      break;
    case t('Undo'):
      array_pop($_SESSION['node_overview_filter']);
      break;
    case t('Reset'):
      $_SESSION['node_overview_filter'] = [];
      break;
  }
}

/**
 * Returns the node administration filters form array to node_admin_content().
 *
 * @see node_admin_nodes()
 * @see node_admin_nodes_submit()
 * @see node_admin_nodes_validate()
 * @see node_filter_form_submit()
 * @see node_multiple_delete_confirm()
 * @see node_multiple_delete_confirm_submit()
 *
 * @ingroup forms
 */
function domain_content_filter_form() {
  $session = isset($_SESSION['node_overview_filter']) ? $_SESSION['node_overview_filter'] : [];
  $filters = domain_content_filters();

  $i = 0;
  $form['filters'] = [
    '#type' => 'fieldset',
    '#title' => t('Show only items where'),
    '#theme' => 'exposed_filters__node',
  ];
  foreach ($session as $filter) {
    [$type, $value] = $filter;
    if ($type == 'term') {
      // Load term name from DB rather than search and parse options array.
      $value = module_invoke('taxonomy', 'term_load', $value);
      $value = $value->name;
    }
    elseif ($type == 'language') {
      $value = $value == LANGUAGE_NONE ? t('Language neutral') : module_invoke('locale', 'language_name', $value);
    }
    else {
      $value = $filters[$type]['options'][$value];
    }
    $t_args = ['%property' => $filters[$type]['title'], '%value' => $value];
    if ($i++) {
      $form['filters']['current'][] = ['#markup' => t('and where %property is %value', $t_args)];
    }
    else {
      $form['filters']['current'][] = ['#markup' => t('where %property is %value', $t_args)];
    }
    if (in_array($type, ['type', 'language'])) {
      // Remove the option if it is already being filtered on.
      unset($filters[$type]);
    }
  }

  $form['filters']['status'] = [
    '#type' => 'container',
    '#attributes' => ['class' => ['clearfix']],
    '#prefix' => ($i ? '<div class="additional-filters">' . t('and where') . '</div>' : ''),
  ];
  $form['filters']['status']['filters'] = [
    '#type' => 'container',
    '#attributes' => ['class' => ['filters']],
  ];
  foreach ($filters as $key => $filter) {
    $form['filters']['status']['filters'][$key] = [
      '#type' => 'select',
      '#options' => $filter['options'],
      '#title' => $filter['title'],
      '#default_value' => '[any]',
    ];
  }

  $form['filters']['status']['actions'] = [
    '#type' => 'actions',
    '#attributes' => ['class' => ['container-inline']],
  ];
  $form['filters']['status']['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => count($session) ? t('Refine') : t('Filter'),
  ];
  if (count($session)) {
    $form['filters']['status']['actions']['undo'] = [
      '#type' => 'submit',
      '#value' => t('Undo'),
    ];
    $form['filters']['status']['actions']['reset'] = [
      '#type' => 'submit',
      '#value' => t('Reset'),
    ];
  }

  backdrop_add_js('core/misc/form.js');

  return $form;
}

/**
 * List node administration filters that can be applied.
 *
 * @return
 *   An associative array of filters.
 */
function domain_content_filters() {
  // Regular filters
  $filters['status'] = [
    'title' => t('status'),
    'options' => [
      '[any]' => t('any'),
      'status-1' => t('published'),
      'status-0' => t('not published'),
      'promote-1' => t('promoted'),
      'promote-0' => t('not promoted'),
      'sticky-1' => t('sticky'),
      'sticky-0' => t('not sticky'),
    ],
  ];
  // Include translation states if we have this module enabled
  if (module_exists('translation')) {
    $filters['status']['options'] += [
      'translate-0' => t('Up to date translation'),
      'translate-1' => t('Outdated translation'),
    ];
  }

  $filters['type'] = [
    'title' => t('type'),
    'options' => [
        '[any]' => t('any'),
      ] + node_type_get_names(),
  ];

  // Language filter if there is a list of languages
  if ($languages = module_invoke('locale', 'language_list')) {
    $languages = [LANGUAGE_NONE => t('Language neutral')] + $languages;
    $filters['language'] = [
      'title' => t('language'),
      'options' => [
          '[any]' => t('any'),
        ] + $languages,
    ];
  }
  return $filters;
}

/**
 * Applies filters for node administration filters based on session.
 *
 * @param $query
 *   A SelectQuery to which the filters should be applied.
 */
function domain_content_build_filter_query(SelectQueryInterface $query) {
  // Build query
  $filter_data = isset($_SESSION['node_overview_filter']) ? $_SESSION['node_overview_filter'] : [];
  foreach ($filter_data as $index => $filter) {
    [$key, $value] = $filter;
    switch ($key) {
      case 'status':
        // Note: no exploitable hole as $key/$value have already been checked when submitted
        [$key, $value] = explode('-', $value, 2);
      case 'type':
      case 'language':
        $query->condition('n.' . $key, $value);
        break;
    }
  }
}

function domain_content_admin_nodes_validate($form, &$form_state) {
  // Error if there are no items to select.
  if (!is_array($form_state['values']['nodes']) || !count(array_filter($form_state['values']['nodes']))) {
    form_set_error('', t('No items selected.'));
  }
}

function domain_content_admin_nodes_submit($form, &$form_state) {
  require_once BACKDROP_ROOT . '/core/modules/node/node.actions.inc';
  require_once BACKDROP_ROOT . '/core/modules/node/node.admin.inc';

  $operations = module_invoke('domain_content', 'node_actions');
  $operation = $operations[$form_state['values']['operation']];
  // Filter out unchecked nodes
  $nodes = array_filter($form_state['values']['nodes']);

  if ($function = $operation['callback']) {
    // Add in callback arguments if present.
    if (isset($operation['callback arguments']) && $operation['callback arguments'] != 'delete') {
      $args = array_merge([$nodes], $operation['callback arguments']);
    }
    else {
      $args = [$nodes];
    }

    if ($operation['callback arguments'] == 'delete') {
      foreach ($args[0] as $arg) {
        node_delete($arg);
      }
    }
    else {

      call_user_func_array($function, $args);
    }

    cache_clear_all();
  }
  else {
    // We need to rebuild the form to go to a second step. For example, to
    // show the confirmation form for the deletion of nodes.
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * Implements hook_node_operations().
 */
function domain_content_node_actions() {
  $operations = [
    'publish' => [
      'label' => t('Publish selected content'),
      'callback' => 'node_mass_update',
      'callback arguments' => ['updates' => ['status' => NODE_PUBLISHED]],
    ],
    'unpublish' => [
      'label' => t('Unpublish selected content'),
      'callback' => 'node_mass_update',
      'callback arguments' => ['updates' => ['status' => NODE_NOT_PUBLISHED]],
    ],
    'promote' => [
      'label' => t('Promote selected content to front page'),
      'callback' => 'node_mass_update',
      'callback arguments' => [
        'updates' => [
          'status' => NODE_PUBLISHED,
          'promote' => NODE_PROMOTED,
        ],
      ],
    ],
    'demote' => [
      'label' => t('Demote selected content from front page'),
      'callback' => 'node_mass_update',
      'callback arguments' => ['updates' => ['promote' => NODE_NOT_PROMOTED]],
    ],
    'sticky' => [
      'label' => t('Make selected content sticky'),
      'callback' => 'node_mass_update',
      'callback arguments' => [
        'updates' => [
          'status' => NODE_PUBLISHED,
          'sticky' => NODE_STICKY,
        ],
      ],
    ],
    'unsticky' => [
      'label' => t('Make selected content not sticky'),
      'callback' => 'node_mass_update',
      'callback arguments' => ['updates' => ['sticky' => NODE_NOT_STICKY]],
    ],
    'node_delete_action' => [
      'type' => 'node',
      'label' => t('Delete content'),
      'callback' => 'node_delete_action',
      'callback arguments' => 'delete',
      'file' => 'node.actions.inc',
    ],
  ];
  return $operations;
}
