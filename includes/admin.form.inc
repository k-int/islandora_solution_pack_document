<?php

/**
 * This file contains all admin functions
 */
module_load_include('inc', 'islandora_document', 'includes/apache-tika-api');
module_load_include('inc', 'islandora_document', 'includes/jodconverter-api');

/**
 * Admin form function.
 */
function islandora_document_admin($form, &$form_state) {
  
  $form = array();

  // Add the file fields.
  islandora_document_admin_add_file_fields($form, $form_state);
  
  // Add the text extraction fields.
  islandora_document_admin_add_text_extraction($form, $form_state);
  
  $form['islandora_document_thumbnail_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Thumbnail'),
    '#description' => t('Settings for document thumbnails'),
  );

  $form['islandora_document_thumbnail_fieldset']['islandora_document_thumbnail_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Width'),
    '#description' => t('The width of the thumbnail in pixels.'),
    '#element_validate' => array('element_validate_number'),
    '#default_value' => variable_get('islandora_document_thumbnail_width', 200),
    '#size' => 5,
  );

  $form['islandora_document_thumbnail_fieldset']['islandora_document_thumbnail_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Height'),
    '#description' => t('The height of the thumbnail in pixels.'),
    '#element_validate' => array('element_validate_number'),
    '#default_value' => variable_get('islandora_document_thumbnail_height', 200),
    '#size' => 5,
  );

  $form['islandora_document_preview_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Preview image'),
    '#description' => t('Settings for document preview image'),
  );

  $form['islandora_document_preview_fieldset']['islandora_document_preview_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Max width'),
    '#description' => t('The maximum width of the preview in pixels.'),
    '#element_validate' => array('element_validate_number'),
    '#default_value' => variable_get('islandora_document_preview_width', 500),
    '#size' => 5,
  );

  $form['islandora_document_preview_fieldset']['islandora_document_preview_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Max height'),
    '#description' => t('The maximum height of the preview in pixels.'),
    '#element_validate' => array('element_validate_number'),
    '#default_value' => variable_get('islandora_document_preview_height', 700),
    '#size' => 5,
  );

  return system_settings_form($form);
}

/**
 * Add text extraction form fields to the form.
 */
function islandora_document_admin_add_text_extraction(&$form, &$form_state) {
	
	// Get the current value.
	$islandora_document_path_to_tika = islandora_document_get_current_form_val ($form_state, 'islandora_document_path_to_tika');
	
	// Default to not found...
	$confirmation_message = '<img src="' . url('misc/watchdog-error.png') . '"/> '
					. t('Please select the location of the tika library.');
	
	if (isset($islandora_document_path_to_tika) && $islandora_document_path_to_tika != '') {
		 
		// Check the version.
		$version = islandora_document_tika_check($islandora_document_path_to_tika);
		 
		if ($version != null) {
			$confirmation_message = '<img src="' . url('misc/watchdog-ok.png') . '"/>'
							. t('!version executable found at !url', array(
									'!url' => "<strong>$islandora_document_path_to_tika</strong>",
									"!version" => $version
							));
		} else {
			$confirmation_message = '<img src="' . url('misc/watchdog-error.png') . '"/> '
							. t('Unable to find Apache Tika executable at !url', array('!url' => "<strong>$islandora_document_path_to_tika</strong>"));
		}
	}
	
	// The fieldset.
	$form['islandora_document_text_fieldset'] = array(
			'#type' => 'fieldset',
			'#title' => t('TEXT EXTRACTION'),
	);
	
	// Checkbox.
	$form['islandora_document_text_fieldset']['islandora_document_extract_text'] = array(
			'#type' => 'checkbox',
			'#title' => t("Extract the text of the document and store?"),
			'#default_value' => variable_get('islandora_document_extract_text', FALSE)
	);
	
	// Ajax message wrapper.
	$form['islandora_document_text_fieldset']['wrapper'] = array(
			'#prefix' => '<div id="islandora-tika-path">',
			'#suffix' => '</div>',
			'#type' => 'markup',
	);
	
	// The path to apache tika element.
	$form['islandora_document_text_fieldset']['wrapper']['islandora_document_path_to_tika'] = array(
			'#type' => 'textfield',
			'#title' => t('Tika jar file'),
			'#size' => 100,
			'#default_value' => variable_get('islandora_document_path_to_tika', ''),
			'#description' => t('The full path to the Apache Tika library <br /> !confirmation_message',
							array(
									'!confirmation_message' => $confirmation_message)
			),
			'#ajax' => array(
					'callback' => 'islandora_update_tika_div',
					'wrapper' => 'islandora-tika-path',
					'effect' => 'fade',
					'event' => 'blur',
					'progress' => array('type' => 'throbber'),
			),
			'#states' => array(
					'visible' => array(
							':input[name="islandora_document_extract_text"]' => array('checked' => TRUE),
					),
			)
	);
}

/**
 * Add the file behaviour fields to the form.
 */
function islandora_document_admin_add_file_fields(&$form, &$form_state) {
	
	// Get the current values.
	$islandora_document_jodc_lib = islandora_document_get_current_form_val ($form_state, 'islandora_document_jodc_lib');
	$islandora_document_jodc_jar = islandora_document_get_current_form_val ($form_state, 'islandora_document_jodc_jar');
	$islandora_document_jodc_port = islandora_document_get_current_form_val ($form_state, 'islandora_document_jodc_port', '2002');
	
	// Default to not found...
	$confirmation_message = '<img src="' . url('misc/watchdog-error.png') . '"/> '
					. t('Please supply the library folder path and the jar archive name for the JODConverter library.');
	
	if (isset($islandora_document_jodc_lib) && $islandora_document_jodc_lib != '' &&
					isset($islandora_document_jodc_jar) && $islandora_document_jodc_jar != '' &&
					isset($islandora_document_jodc_port) && $islandora_document_jodc_port != '') {

		// Need to add the trailing slash if not present.
		$lastChar = $islandora_document_jodc_lib{(strlen($islandora_document_jodc_lib) - 1)};
		if ($lastChar != DIRECTORY_SEPARATOR) {
			$islandora_document_jodc_lib .= DIRECTORY_SEPARATOR;
			$form_state['values']['islandora_document_jodc_lib'] = $islandora_document_jodc_lib;
			variable_set('islandora_document_jodc_lib', $islandora_document_jodc_lib);
		}
		
		// No version option available in jodconverter so just try the test method.			
		if (islandora_document_jodc_check($islandora_document_jodc_lib, $islandora_document_jodc_jar, $islandora_document_jodc_port)) {
			$confirmation_message = '<img src="' . url('misc/watchdog-ok.png') . '"/>'
							. t('JODConverter executable found at !url', array(
									'!url' => "<strong>$islandora_document_jodc_lib$islandora_document_jodc_jar</strong>"
							));
		} else {
			$confirmation_message = '<img src="' . url('misc/watchdog-error.png') . '"/> '
							. t('Unable to find JODConverter executable at !url', array('!url' => "<strong>$islandora_document_jodc_lib/$islandora_document_jodc_jar</strong>"));
		}
	}
	
	// Fieldset.
	$form['islandora_document_file_fieldset'] = array(
			'#type' => 'fieldset',
			'#title' => t('File'),
	);
	
	// Allowed file types.
	$form['islandora_document_file_fieldset']['islandora_document_allowed_types'] = array(
			'#type' => 'textfield',
			'#title' => t('Allowed file extensions'),
			'#description' => t('Enter a list of allowed file extensions separated by a comma. Leave blank to not limit by extension.'),
			'#default_value' => variable_get('islandora_document_allowed_types', ''),
			'#size' => 100,
	);
	
	// Fieldset.
	$form['islandora_document_jodc_fieldset'] = array(
			'#type' => 'fieldset',
			'#title' => t('JODConverter'),
	);
	
	// Wrapper for AJAX callback.
	$form['islandora_document_jodc_fieldset']['wrapper'] = array(
			'#prefix' => '<div id="islandora-jodc">',
			'#suffix' => '</div>',
			'#type' => 'markup',
	);
	
	// The path to apache tika element.
	$form['islandora_document_jodc_fieldset']['wrapper']['islandora_document_jodc_lib'] = array(
			'#type' => 'textfield',
			'#title' => t('JODConverter lib directory.'),
			'#size' => 100,
			'#description' => t("The full path to the JODConverter lib directory."),
			'#default_value' => $islandora_document_jodc_lib,
			'#ajax' => array(
					'callback' => 'islandora_update_jodc_div',
					'wrapper' => 'islandora-jodc',
					'effect' => 'fade',
					'event' => 'blur',
					'progress' => array('type' => 'throbber'),
			),
	);
	
	$form['islandora_document_jodc_fieldset']['wrapper']['islandora_document_jodc_jar'] = array(
			'#type' => 'textfield',
			'#title' => t('JODConverter executable jar name.'),
			'#size' => 100,
			'#description' => t("The name of the JODConverter executable jar file within the above lib directory."),
			'#default_value' => $islandora_document_jodc_jar,
			'#ajax' => array(
					'callback' => 'islandora_update_jodc_div',
					'wrapper' => 'islandora-jodc',
					'effect' => 'fade',
					'event' => 'blur',
					'progress' => array('type' => 'throbber'),
			)
	);
	
	$form['islandora_document_jodc_fieldset']['wrapper']['islandora_document_jodc_port'] = array(
			'#type' => 'textfield',
			'#title' => t('JODConverter port.'),
			'#size' => 100,
			'#description' => t('The port number JODConverter uses to connect to OpenOffice/LibreOffice runnning in headless mode.<br /> !confirmation_message',
							array(
									'!confirmation_message' => $confirmation_message)
			),
			'#default_value' => $islandora_document_jodc_port,
			'#ajax' => array(
					'callback' => 'islandora_update_jodc_div',
					'wrapper' => 'islandora-jodc',
					'effect' => 'fade',
					'event' => 'blur',
					'progress' => array('type' => 'throbber'),
			)
	);
}

/**
 * Get current value of a field.
 */
function islandora_document_get_current_form_val(&$form_state, $name, $default = '') {
	if (isset($form_state['values'][$name])) {
		$value = $form_state['values'][$name];
	}
	else {
		$value = variable_get($name, $default);
	}
	
	return $value;
}


/**
 * Admin form validate function
 */
function islandora_document_admin_validate($form, &$form_state) {
	$val = islandora_document_get_current_form_val ($form_state, 'islandora_document_jodc_lib');
	$val .= islandora_document_get_current_form_val ($form_state, 'islandora_document_jodc_jar');
	$val .= islandora_document_get_current_form_val ($form_state, 'islandora_document_jodc_port');
	
	if ($val == '') {
		// No JODConverter setup.
		form_set_error('islandora_document_jodc_fieldset', t('You must enter the JODConverter details.'));
	}
	
	// Extract text is checked but tika has not been configured.
  if ($form_state['values']['islandora_document_extract_text']) {
    if (islandora_document_tika_check($form_state['values']['islandora_document_path_to_tika']) == null) {
    	form_set_error('islandora_document_path_to_tika', t('Cannot extract text from document without a valid path to Apache Tika.'));
    }
  }
}

/**
 * Check to see if the tika api can be initialised by the path specified.
 */
function islandora_document_tika_check($path) {
	
	// Just get the version.
	$tika = new TikaAPI($path);
	return $tika->getVersion();
}

/**
 * Check to see if JODConverter api can be initialised by the path specified.
 */
function islandora_document_jodc_check($libdir, $jar, $port) {

	// Just get the version.
	$jodc = new JodCAPI($libdir, $jar, $port);
	return $jodc->test();
}

/**
 * Callback function to return a status message to the admin forms.
 */
function islandora_update_tika_div($form, $form_state) {
  unset($form_state['submit_handlers']);
  $form_state['rebuild'] = TRUE;

  return $form['islandora_document_text_fieldset']['wrapper'];
}

/**
 * Callback function to return a status message to the admin forms.
 */
function islandora_update_jodc_div($form, $form_state) {
	unset($form_state['submit_handlers']);
	$form_state['rebuild'] = TRUE;

	return $form['islandora_document_jodc_fieldset']['wrapper'];
}