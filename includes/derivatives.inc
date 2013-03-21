<?php

/**
 * @file
 *
 * Contains all the functions required for adding derived datastreams from the uploaded file for the document content model.
 */

/**
 * Creates all the derivatives for this content model type
 *
 * @param FedoraObject $object
 *   The object to generate/add the deriviative datastreams to.
 *
 * @return boolean
 *   TRUE if all derivatives were created successfully, FALSE otherwise.
 */
function islandora_document_create_all_derivatives(FedoraObject $object) {
  if (!isset($object['OBJ'])) {
    drupal_set_message(t('Could not create derivatives for %s.  No file was uploaded.', array('%s' => $object->id)), 'error');
    return FALSE;
  }
  $success = TRUE;
  $file_uri = islandora_document_get_derivative_source_as_unmanaged_file($object);
  
  // Get the mime type.
  $mime_detector = new MimeDetect();
  $mimetype = $mime_detector->getMimetype($file_uri);
  
  // First derive a PDF if needed.
  if ($mimetype != $mime_detector->getMimetype("pdf")) {
  	$pdf_file = islandora_document_add_pdf_derivative($object, $file_uri);
  } else {
  	$pdf_file = $file_uri;
  }
  
  // Create the Images based on the PDF.
  if ($pdf_file != FALSE) {
	  $success &= islandora_document_add_tn_derivative($object, $pdf_file);
	  $success &= islandora_document_add_preview_derivative($object, $pdf_file);
  }

  // Extract the text from the original.
  if (!isset($object['EXTRACTED_TEXT']) && variable_get('islandora_document_extract_text', FALSE)) {
    $success &= islandora_document_add_text_derivative($object, $file_uri);
  }
//   file_unmanaged_delete($file_uri);
  return $success;
}

/**
 * Gets the source of all derivatives as an unmanaged file, deletion of the file is left to the caller.
 *
 * @param FedoraObject $object
 *   The object to generate/add the deriviative datastreams to.
 *
 * @return string
 *   The file path to the unmanaged derivative source file.
 */
function islandora_document_get_derivative_source_as_unmanaged_file(FedoraObject $object) {
  $mime_detect = new MimeDetect();
  $ext = $mime_detect->getExtension($object['OBJ']->mimeType);
  $file_name = str_replace(':', '-', $object->id);
  $file_uri = file_create_filename("{$file_name}.OBJ.{$ext}", 'temporary://');
  $object['OBJ']->getContent($file_uri);
  return $file_uri;
}

/**
 * Creates a JPG deriviative for the TN datastream.
 *
 * @param FedoraObject $object
 *   The object to add the deriviative datastreams to.
 * @param string $file_uri
 *   The URI to the document file the derivative will be generated from.
 *
 * @return boolean
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_document_add_tn_derivative(FedoraObject $object, $file_uri) {
  $width = variable_get('islandora_document_thumbnail_width', 200);
  $height = variable_get('islandora_document_thumbnail_height', 200);
  return islandora_document_add_jpg_derivative($object, $file_uri, 'TN', $width, $height);
}

/**
 * Tries to create a PDF derivitive for optimal document portability.
 *
 * @param FedoraObject $object
 *   The object to add the deriviative datastreams to.
 * @param string $file_uri
 *   The URI to the document file the derivative will be generated from.
 *
 * @return boolean
 *   The filename of the PDF so it can be used with Thumb generation, or NULL if failed.
 */
function islandora_document_add_pdf_derivative(FedoraObject $object, $file_uri) {
	$dsid = "PDF_ALT";
  $source = drupal_realpath($file_uri);
  
  // Get JODConverter API and try and create a PDF.
  $pdf_file = islandora_document_create_pdf_derivative($source, $dsid);
  
  if ($pdf_file == FALSE) {
  	
  	// Error.
    drupal_set_message(t('Failed to create PDF derivitive for @dsid.', array('@dsid' => $dsid)), 'error');
  } else {
  	
  	// Add the datastream.
  	islandora_document_add_datastream($object, $dsid, $pdf_file);
  }
  
  return $pdf_file;
}

function islandora_document_create_pdf_derivative($file_uri, $dsid) {
	$source = drupal_realpath($file_uri);
	$matches = array();
	preg_match("/\/([^.]*).*$/", $source, $matches); // Get the base name of the source file.
	$temp = drupal_tempnam("temporary://", "{$matches[1]}.$dsid") . ".pdf";
	$dest = drupal_realpath($temp);

	// Get the JODConverter API.
	module_load_include('inc', 'islandora_document', 'includes/jodconverter-api');
	$JODC = new JodCAPI();
	
	// Convert the document.
	if ($JODC->convertToPDF($source, $dest) != 0) {
		$message = 'Failed to create PDF derivative of @source at @dest';
		$variables = array('@source' => $source, '@dest' => $dest);
		watchdog('islandora_document', $message, $variables, WATCHDOG_ERROR);
		return FALSE;
	}
	
	return $dest;
}

/**
 * Creates a textural deriviative for the EXTRACTED_TEXT datastream.
 *
 * @param FedoraObject $object
 *   The object to add the deriviative datastreams to.
 * @param string $file_uri
 *   The URI to the document file the derivative will be generated from.
 *
 * @return boolean
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_document_add_text_derivative(FedoraObject $object, $file_uri) {
  $dsid = "EXTRACTED_TEXT";
  $source = drupal_realpath($file_uri);
  $success = FALSE;
  
  // Get the Tika API and extract the text.
  module_load_include('inc', 'islandora_document', 'includes/apache-tika-api');
  $tika = new TikaAPI();
  $success = islandora_document_add_text_datastream($object, $dsid, $tika->getText($source));
  
  if ($success == FALSE) {
    drupal_set_message(t('Failed to extract text for @dsid.', array('@dsid' => $dsid)), 'error');
  }
  return $success;
}

/**
 * Creates a JPG deriviative for the PREVIEW datastream.
 *
 * @param FedoraObject $object
 *   The object to add the deriviative datastreams to.
 * @param string $file_uri
 *   The URI to the document file the derivative will be generated from.
 *
 * @return boolean
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_document_add_preview_derivative(FedoraObject $object, $file_uri) {
  $width = variable_get('islandora_document_preview_width', 500);
  $height = variable_get('islandora_document_preview_height', 700);
  return islandora_document_add_jpg_derivative($object, $file_uri, 'PREVIEW', $width, $height);
}

/**
 * Creates a JPG deriviative of the given file.
 *
 * @param FedoraObject $object
 *   The object to add the deriviative datastreams to.
 * @param string $file_uri
 *   The URI to the document file the derivative will be generated from.
 * @param string $dsid
 *   The datastream ID to use for the derived datastream.
 * @param integer $width
 *   The width to make the derived datastream.
 * @param integer $height
 *   The height to make the derived datastream.
 *
 * @return boolean
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_document_add_jpg_derivative(FedoraObject $object, $file_uri, $dsid, $width, $height) {
  $success = FALSE;
  $derivative_file_uri = islandora_document_create_jpg_derivative($file_uri, $dsid, $width, $height);
  if ($derivative_file_uri !== FALSE) {
    $success = islandora_document_add_datastream($object, $dsid, $derivative_file_uri);
    file_unmanaged_delete($derivative_file_uri);
  }
  if ($success == FALSE) {
    drupal_set_message(t('Failed to create @dsid derivative.', array('@dsid' => $dsid)), 'error');
  }
  return $success;
}

/**
 * Creates a jpg derivative from the given document file.
 *
 * @param string $file_uri
 *   The URI to the document file the derivative will be generated from.
 * @param string $dsid
 *   Used to identify the generated file.
 * @param integer $width
 *   The width to make the derived datastream.
 * @param integer $height
 *   The height to make the derived datastream.
 *
 * @return string
 *   A URI to the generated derivative if successful, FALSE otherwise.
 */
function islandora_document_create_jpg_derivative($file_uri, $dsid, $width, $height) {
  $source = drupal_realpath($file_uri) . '[0]';
  $matches = array();
  preg_match("/\/([^.]*).*$/", $source, $matches); // Get the base name of the source file.
  $temp = drupal_tempnam("temporary://", "{$matches[1]}.$dsid.jpg");
  $dest = drupal_realpath($temp);
  $args['quality'] = '-quality ' . escapeshellarg(variable_get('imagemagick_quality', 75));
  $args['previewsize'] = '-resize ' . escapeshellarg("{$width}x{$height}");
  $args['colors'] = '-colorspace RGB';
  
  // PDFs with transparency in them will cause odd results when converting,
  // to JPG as there is no alpha channel in the JPG format. To correct this,
  // we should flatten the image to remove the alpha channel.
  $args['background'] = '-flatten';
  $context = array(
    'source' => $source,
    'destination' => $dest,
  );
  drupal_alter('imagemagick_arguments', $args, $context);
  // To make use of ImageMagick 6's parenthetical command grouping we need to make
  // the $source image the first parameter and $dest the last.
  // See http://www.imagemagick.org/Usage/basics/#cmdline
  $command = escapeshellarg($source) . ' ' . implode(' ', $args) . ' ' . escapeshellarg("jpg:$dest");
  $output = '';
  $ret = -1;
  if (_imagemagick_convert_exec($command, $output, $ret) !== TRUE) {
    $message = 'imagemagick convert failed to create derivative<br/>Error: @ret<br/>Command: @command<br/>Output: @output';
    $variables = array('@ret' => $ret, '@command' => $command, '@output' => $output);
    watchdog('islandora_document', $message, $variables, WATCHDOG_ERROR);
    return FALSE;
  }
  return $dest;
}

/**
 * Adds the given file as a datastream to the given object using the given datastream id to identify it.
 *
 * @param FedoraObject $object
 *   The object to add the datasteam to.
 * @param string $datastream_id
 *   The datastream id of the added datastream.
 * @param string $file_uri
 *   A URI to the file containing the content for the datastream.
 */
function islandora_document_add_datastream($object, $datastream_id, $file_uri) {
  try {
    $mime_detector = new MimeDetect();
    $ds = $object->constructDatastream($datastream_id, "M");
    $ds->label = $datastream_id;
    $ds->mimetype = $mime_detector->getMimetype($file_uri);
    $ds->setContentFromFile(drupal_realpath($file_uri), FALSE);
    $object->ingestDatastream($ds);
    return TRUE;
  } catch (exception $e) {
    drupal_set_message(t("@message", array("@message" => $e->getMessage())));
    return FALSE;
  }
}

function islandora_document_add_text_datastream(FedoraObject $object, $datastream_id, $text) {
	try {
		$ds = $object->constructDatastream($datastream_id, "M");
		$ds->label = $datastream_id;
		$ds->mimetype = 'text/plain';
		$ds->setContentFromString($text);
		$object->ingestDatastream($ds);
		return TRUE;
	} catch (exception $e) {
		drupal_set_message(t("@message", array("@message" => $e->getMessage())));
		return FALSE;
	}
}
