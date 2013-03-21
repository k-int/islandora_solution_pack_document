<?php

/**
 * @file
 *
 * Handles the uploading of the document file as part of the ingest process.
 */

/**
 * Defines a file upload form for uploading the document file.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 * @return
 *   The drupal form definition.
 */
function islandora_document_document_upload_form(array $form, array &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = variable_get('islandora_document_allowed_types', NULL);
  $form = array();
  $form['file'] = array(
    '#title' => t('Document'),
    '#type' => 'managed_file',
    '#required' => TRUE,
    '#description' => t('Select file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions)),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_size' => array($upload_size * 1024 * 1024), // Assume its specified in MB
    )
  );
  
  // Validate extension if present.
  if ($extensions != NULL) {
  	$form['file']['#upload_validators']['file_validate_extensions'] = array($extensions);
  }
  return $form;
}

/**
 * Adds the uploaded file into the ingestable objects 'OBJ' datastream.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_document_document_upload_form_submit(array $form, array &$form_state) {
  $object = islandora_ingest_form_get_object($form_state);
  
  if (empty($object['OBJ'])) {
    $ds = $object->constructDatastream('OBJ', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['OBJ'];
  }
  $document_file = file_load($form_state['values']['file']);
  $document_path = drupal_realpath($document_file->uri);
  $ds->setContentFromFile($document_path, FALSE);
  $ds->label = $document_file->filename;
  $ds->mimetype = $document_file->filemime;
}
