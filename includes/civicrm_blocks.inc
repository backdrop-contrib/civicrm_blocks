<?php


/**
 *  CivicrmContributionBlock extends Block
 *
 *  This class allows us to create blocks from CiviCRM Contribution Pages.
 */
class CivicrmContributionBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'contribution_page' => '',
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
        if (!empty($this->settings['contribution_page'])) {
          civicrm_initialize();
          $contributionpage = civicrm_api3('ContributionPage', 'getsingle', [
            'sequential' => 1,
            'is_active' => 1,
            'id' => $this->settings['contribution_page'],
          ]);
          $title = $contributionpage['title'];
        }
        break;
    }
    return $title;
  }
  
  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('CiviCRM Contribution Page');
  }
  
  /**
   * Returns the rendered content of this block.
   *
   * If there is no content, we'll return nothing.
   *
   * @return string
   */
  function getContent() {
    if (!empty($this->settings['contribution_page'])) {
      civicrm_initialize();
      $contributionpage = civicrm_api3('ContributionPage', 'getsingle', [
        'sequential' => 1,
        'is_active' => 1,
        'id' => $this->settings['contribution_page'],
      ]);
      if (isset($contributionpage['id']) && isset($contributionpage['is_active']) && $contributionpage['is_active'] == '1') {
        // Initialize the system by creating a config object
        $config = CRM_Core_Config::singleton();
        $config->userFrameworkFrontend = TRUE;
  
        // Load core resources
        CRM_Core_Resources::singleton()->addCoreResources();
  
        $args = array(
          'id' => $contributionpage['id'],
          'reset' => 1,
          'q' => 'civicrm/contribute/transact',
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
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    civicrm_initialize();
    $activepages = civicrm_api3('ContributionPage', 'get', [
      'sequential' => 1,
      'is_active' => 1,
    ]);
    if ($activepages['count'] > 0) {
      $page_options = array();
      foreach ($activepages['values'] as $activepage) {
        $page_options[$activepage['id']] = $activepage['title'];
      }
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
      $form['contribution_page'] = array(
        '#type' => 'select',
        '#title' => t('CiviCRM Contribution Page'),
        //TODO: populate with active page values
        '#options' => $page_options,
        '#default_value' => $this->settings['contribution_page'],
        '#description' => t('Which CiviCRM Contribution page would you like to display?'),
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
    else {
      $form['contribution_page'] = array(
        '#markup' => t('There are no active CiviCRM Contribution Pages currently. Please set one up first.'),
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
      $this->settings['contribution_page'] = $form_state['values']['contribution_page'];
    }
  }
}