<?php
/**
 * @file
 * Contains the definition of the CustomAccess plugin for domain_blocks.
 */

/**
 * Defines a custom access condition for layouts.
 */
class DomainLayoutAccess extends LayoutAccess {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'domain_list' => array(),
    );
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    return t('Custom condition: Domain Blocks Specific Condition');
  }

  /**
   * Checks if the current user meets the access conditions.
   */
  public function checkAccess() {
    // Get the current domain ID.
    $current_domain_id = domain_get_domain()['domain_id'];

    // Check if domain is selected.
    if (!empty($this->settings['domain']) && isset($this->settings['domain'])) {

      // Check if if the block is available on current domain.
      if (in_array($current_domain_id, $this->settings['domain'])) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      return TRUE;
    }
  }

  /**
   * Provides the configuration form for the access condition.
   */
  public function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $options = array();
    $format = domain_select_format();
    foreach (domain_domains() as $data) {
      $key = $data['domain_id'];
      // The domain must be valid or accessible for the current user
      if ($data['valid'] || user_access('administer domains') || user_access('access inactive domains')) {
        $options[$key] = $data['sitename'];
      }
    }
    $form['domain'] = array(
      '#title' => t('Domain'),
      '#type' => 'checkboxes',
      '#options' => $options,
      '#description' => t('Limit block display to selected affiliates. When no affiliates are selected this block is visible on all domains by default.'),
      '#default_value' => $this->settings['domain'],
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $this->settings['domain'] = array_keys(array_filter($form_state['values']['domain']));
  }
}
