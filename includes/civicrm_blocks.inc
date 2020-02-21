<?php


/**
 *  CivicrmPageBlock extends Block
 *
 *  This class allows us to create blocks from CiviCRM Pages.
 */
class CivicrmPageBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'contribution_page' => '',
      'event_page' => '',
    );
    $this->settings['contexts']['civicrm-blocks'] = array(
      'title' => 'Configure this CiviCRM Page',
      'href' => url('/civicrm/admin/contribute?reset=1', array('absolute' => TRUE)),
    );
  }
  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    $title = '';
    switch ($this->settings['title_display']) {
      case LAYOUT_TITLE_CUSTOM:
        $title = isset($this->settings['title']) ? check_plain($this->settings['title']) : '';
        break;
      case LAYOUT_TITLE_NONE:
        break;
      case LAYOUT_TITLE_DEFAULT:
      default:
        if (!empty($this->settings['contribution_page']) || !empty($this->settings['event_page'])) {
          civicrm_initialize();
          switch ($this->settings['page_type']) {
            case 'civicrm/contribute/transact':
              $contributionpage = civicrm_api3('ContributionPage', 'getsingle', [
                'sequential' => 1,
                'is_active' => 1,
                'id' => $this->settings['contribution_page'],
              ]);
              if (isset($contributionpage['id']) && isset($contributionpage['is_active']) && $contributionpage['is_active'] == '1') {
                $title = $contributionpage['title'];
              }
              break;
              
            case 'civicrm/event/register':
              $eventpage = civicrm_api3('Event', 'getsingle', [
                'sequential' => 1,
                'is_public' => 1,
                'start_date' => ['>=' => date('Y-m-d')],
                'end_date' => ['>=' => date('Y-m-d')],
                'is_online_registration' => 1,
                'id' => $this->settings['event_page'],
              ]);
              if (isset($eventpage['id']) && isset($eventpage['is_active']) && $eventpage['is_active'] == '1') {
                $title = $eventpage['title'];
              }
              break;
              
            default:
              break;
          }
        }
        break;
    }
    return $title;
  }
  
  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('CiviCRM Page');
  }
  
  /**
   * Returns the rendered content of this block.
   *
   * If there is no content, we'll return nothing.
   *
   * @return string
   */
  function getContent() {
    $output = '';
    if (!empty($this->settings['contribution_page']) || !empty($this->settings['event_page'])) {
      civicrm_initialize();
      switch ($this->settings['page_type']) {
        case 'civicrm/contribute/transact':
          $contributionpage = civicrm_api3('ContributionPage', 'getsingle', [
            'sequential' => 1,
            'is_active' => 1,
            'id' => $this->settings['contribution_page'],
          ]);
          if (isset($contributionpage['id']) && isset($contributionpage['is_active']) && $contributionpage['is_active'] == '1') {
            $output = _civicrm_blocks_inject_page($this->settings['page_type'], $this->settings['contribution_page']);
          }
          break;
          
        case 'civicrm/event/register':
          $eventpage = civicrm_api3('Event', 'getsingle', [
            'sequential' => 1,
            'is_public' => 1,
            'start_date' => ['>=' => date('Y-m-d')],
            'end_date' => ['>=' => date('Y-m-d')],
            'is_online_registration' => 1,
            'id' => $this->settings['event_page'],
          ]);
          if (isset($eventpage['id']) && isset($eventpage['is_active']) && $eventpage['is_active'] == '1') {
            $output = _civicrm_blocks_inject_page($this->settings['page_type'], $this->settings['event_page']);
          }
          break;
          
        default:
          break;
      }
    }
    return $output;
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    civicrm_initialize();
    parent::form($form, $form_state);
    $form['title_display']['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom title'),
      '#default_value' => $this->settings['title'],
      '#states' => array(
        'visible' => array(
          'form.layout-block-configure-form :input[name="title_display"]' => array('value' => LAYOUT_TITLE_CUSTOM),
        ),
      ),
      '#maxlength' => 255,
    );
    $form['page_type'] = array(
      '#type' => 'select',
      '#title' => t('CiviCRM Page Type'),
      '#default_value' => isset($this->settings['page_type']) ? $this->settings['page_type'] : NULL,
      '#options' => _civicrm_blocks_pagetype(),
    );
    $form['contribution_page'] = array(
      '#type' => 'select',
      '#title' => t('Contribution Page'),
      '#default_value' => isset($this->settings['contribution_page']) ? $this->settings['contribution_page'] : NULL,
      '#options' => _civicrm_blocks_contribution_page_options(),
      '#states' => array(
        'visible' => array(
          'select[name="page_type"]' => array('value' => 'civicrm/contribute/transact'),
        ),
      ),
    );
    $form['event_page'] = array(
      '#type' => 'select',
      '#title' => t('Event Registration Page'),
      '#default_value' => isset($this->settings['event_page']) ? $this->settings['event_page'] : NULL,
      '#options' => _civicrm_blocks_event_page_options(),
      '#states' => array(
        'visible' => array(
          'select[name="page_type"]' => array('value' => 'civicrm/event/register'),
        ),
      ),
    );
    if (module_exists('block')) {
      $form['convert'] = array(
        '#weight' => 2,
        '#parents' => array(),
      );
      $form['convert']['reusable'] = array(
        '#type' => 'checkbox',
        '#title' => t('Make this block reusable'),
        '#description' => t('If enabled, this block will be made reusable across different layouts and be listed in on the !block_link page.', array('!block_link' => l(t('Custom blocks'), 'admin/structure/block'))),
        '#weight' => 1,
      );
      $form['convert']['label'] = array(
        '#type' => 'textfield',
        '#title' => t('Admin label'),
        '#maxlength' => 64,
        '#description' => t('This text is used only in administrative interfaces. It will not be shown to site visitors.'),
        '#weight' => 2,
        '#states' => array(
          'visible' => array(
            ':input[name="reusable"]' => array('checked' => TRUE),
          ),
        ),
      );
      $form['convert']['description'] = array(
       '#type' => 'textfield',
       '#title' => t('Admin description'),
       '#maxlength' => 128,
       '#description' => t('This text is used only in administrative interfaces. It will not be shown to site visitors.'),
       '#weight' => 4,
        '#states' => array(
          'visible' => array(
            ':input[name="reusable"]' => array('checked' => TRUE),
          ),
        ),
      );
      $form['convert']['delta'] = array(
        '#type' => 'machine_name',
        '#title' => t('Internal name'),
        '#maxlength' => 64,
        '#machine_name' => array(
          'source' => array('convert', 'label'),
          'exists' => 'block_custom_block_load',
        ),
        '#description' => t('A unique machine-readable name containing letters, numbers, and underscores.'),
        '#weight' => 3,
        '#required' => FALSE,
      );
    }
  }
  /**
   * Validates the form settings.
   */
  function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);
    if (module_exists('block')) {
      $label = trim($form_state['values']['label']);
      $delta = trim($form_state['values']['delta']);
      if ($form_state['values']['reusable']) {
        if (empty($label)) {
          form_error($form['convert']['label'], t('Admin label is required when making a block reusable.'));
        }
        elseif (empty($delta)) {
          form_error($form['convert']['delta'], t('An internal name is required when making a block reusable.'));
        }
      }
    }
  }
  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    if (module_exists('block') && $form_state['values']['reusable']) {
      $delta = $form_state['values']['delta'];
      $this->settings = array_diff_key($this->settings, array('title' => '', 'contribution_page' => ''));
      $this->plugin = 'block:' . $delta;
      $edit = array(
        'info' => $form_state['values']['label'],
        'title' => $form_state['values']['title'],
        'description' => $form_state['values']['description'],
      );
      block_custom_block_save($edit, $delta);
    }
    else {
      $this->settings['title'] = $form_state['values']['title'];
      $this->settings['page_type'] = $form_state['values']['page_type'];
      $this->settings['contribution_page'] = $form_state['values']['contribution_page'];
      $this->settings['event_page'] = $form_state['values']['event_page'];
    }
  }
}

/**
 * Page types currently supported.
 *
 */
function _civicrm_blocks_pagetype($type = NULL) {
  civicrm_initialize();
  static $q;

  if (empty($q)) {
    $q = array(
      'civicrm/event/register' => 'Event Registration Page',
      'civicrm/contribute/transact' => 'Contribution Page',
    );
  }

  if ($type) {
    return isset($q[$type]) ? $q[$type] : '';
  }

  return $q;
}


/**
 * Currently active contribution pages.
 *
 */
function _civicrm_blocks_contribution_page_options() {
  civicrm_initialize();
  $contribution_page_options = array();
  $contribution_activepages = civicrm_api3('ContributionPage', 'get', [
    'sequential' => 1,
    'is_active' => 1,
  ]);
  if ($contribution_activepages['count'] > 0) {
    $contribution_page_options[0] = 'None';
    foreach ($contribution_activepages['values'] as $contribution_activepage) {
      $contribution_page_options[$contribution_activepage['id']] = $contribution_activepage['title'];
    }
  }
  return $contribution_page_options;
}

/**
 * Currently active event pages.
 *
 */
function _civicrm_blocks_event_page_options() {
  civicrm_initialize();
  $event_page_options = array();
  $event_activepages = civicrm_api3('Event', 'get', [
    'sequential' => 1,
    'is_public' => 1,
    'start_date' => ['>=' => date('Y-m-d')],
    'end_date' => ['>=' => date('Y-m-d')],
    'is_online_registration' => 1,
  ]);
  if ($event_activepages['count'] > 0) {
    $event_page_options[0] = 'None';
    foreach ($event_activepages['values'] as $event_activepage) {
      $event_page_options[$event_activepage['id']] = $event_activepage['title'];
    }
  }
  return $event_page_options;
}

/**
  * Inject CiviCRM page into content.
  *
  */
function _civicrm_blocks_inject_page($type = '', $pageid = 0) {
  if (_civicrm_blocks_pagetype($type) != '' && is_numeric($pageid) && $pageid > 0) {
    civicrm_initialize();
    // Initialize the system by creating a config object
    $config = CRM_Core_Config::singleton();
    $config->userFrameworkFrontend = TRUE;

    // Load core resources
    CRM_Core_Resources::singleton()->addCoreResources();

    $args = array(
      'id' => $pageid,
      'reset' => 1,
      'q' => $type,
    );
    
    // invoke() requires environment variables to be set
    $request = $_REQUEST;
    $get = $_GET;
    $pagetitle = backdrop_get_title();
    foreach ( $args as $key => $value ) {
      if ( $value !== NULL ) {
        $_REQUEST[$key] = $_GET[$key] = $value;
      }
    }
    
    //Invoke will require the 'q' arg to be broken up.
    $explodedargs = explode('/', $args['q']);
    $args = array_merge($explodedargs, $args);
    
    ob_start(); // Start buffering
    CRM_Core_Invoke::invoke($args); // Save output into buffer
    $content = ob_get_clean(); // Save the buffered output and flush the buffer
    
    // reset the environment
    $_REQUEST = $request;
    $_GET = $get;
    backdrop_set_title($pagetitle);
    
    return $content;
  }
}
