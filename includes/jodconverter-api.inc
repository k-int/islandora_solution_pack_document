<?php

/**
 * The global tika api for use throughout the solution pack.
 * @author sosguthorpe
 *
 */

class JodCAPI {
	private $lib_path;
	private $path;
	private $port;
		
	public function __construct($possible_libpath = NULL, $jar_name = NULL, $port = NULL) {
		$this->lib_path = 
			$possible_libpath != NULL ? $possible_libpath : realpath(variable_get('islandora_document_jodc_lib', NULL));
		$lastChar = $this->lib_path{(strlen($this->lib_path) - 1)};
		if ($lastChar != DIRECTORY_SEPARATOR) {
			$this->lib_path .= DIRECTORY_SEPARATOR;
		}
		$this->path = $this->lib_path .
			($jar_name != NULL ? $jar_name :  variable_get('islandora_document_jodc_jar', NULL));
		$this->port = 
			$port != NULL ? $port : variable_get('islandora_document_jodc_port', '2002');
	}
	
	public function convertToPDF($source, $dest) {
		return $this->_execute(
			$source,
			$dest
		);
	}
	
	public function test() {
		$result = $this->_execute();		
		return $this->_execute() != 0;
	}
	
	private function _execute($args = array()) {
		
		// Return NULL if not set.
		if ($this->path == NULL || $this->lib_path == NULL || $this->port == NULL) return NULL;
		
		// Build the basic command based on this path.
		$cmd = escapeshellcmd('java') . ' ' .
			escapeshellarg('-Dfile.encoding=UTF8') .
			' -cp ' . escapeshellarg($this->lib_path) .
			' -jar ' . escapeshellarg($this->path) .
			' --port ' . escapeshellarg($this->port)
		;
		
		// Add each argument passed to this method.
		for ($i = 0; $i < func_num_args(); $i++) {
			$cmd .= ' ' . escapeshellarg(func_get_arg($i));
		}
		
		// Return value doesn't really work here. Use the return code instead.
		$return_var = 0;
		$output = array();
		exec($cmd, $output, $return_var);
		
		// Return the result.
		return $return_var;
	}
}