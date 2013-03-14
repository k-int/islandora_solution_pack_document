<?php

/**
 * The global tika api for use throughout the solution pack.
 * @author sosguthorpe
 *
 */

class TikaAPI {
	private $path;
		
	public function __construct($possible_path = null) {
		$this->path = realpath(variable_get('islandora_document_path_to_tika', $possible_path));
	}
	
	public function getVersion () {
		return $this->_execute ("--version");
	}
	
	public function getText($file) {
		return $this->_execute(
			"--text",
			$file
		);
	}
	
	public function getMetadata($file) {
		return $this->_execute(
			"--metadata",
			$file
		);
	}
	
	private function _execute($args) {
		
		// Return null if not set.
		if ($this->path == null) return null;
		
		// Build the basic command based on this path.
		$cmd = escapeshellcmd('java') . ' ' .
			escapeshellarg('-Dfile.encoding=UTF8') .
			' -jar ' . escapeshellarg($this->path) .
			' --encoding=' . escapeshellarg('UTF8')
		;
		
		// Add each argument passed to this method.
		for ($i = 0; $i < func_num_args(); $i++) {
			$cmd .= ' ' . escapeshellarg(func_get_arg($i));
		}
		
		// Return the result.
		return shell_exec($cmd);
	}
}