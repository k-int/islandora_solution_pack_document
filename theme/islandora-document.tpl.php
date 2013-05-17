<?php

/**
 * @file
 * This is the template file for the document object
 *
 * @TODO: Add documentation about this file and the available variables
 */
?>

<div class="islandora-document-object islandora">
  <div class="islandora-document-content-wrapper clearfix">
    <?php if (isset($islandora_content)): ?>
      <div class="islandora-document-content">
        <?php print $islandora_content; ?>
      </div>
      <?php if (sizeof($downloadables) > 0): ?>
      	<h2>Downloads </h2>
      	<ul><?php foreach ($downloadables as $name => $url) : ?>
      		<li><?php 
      			$pinfo = pathinfo($name);
      			print	l($pinfo['extension'], $url,
							array('attributes' => array(
									'class' => array('islandora-document-link'),
									'title' => $name
							))
						); ?>
					</li>
			<?php endforeach; ?></ul></div>
      <?php endif; ?>
    <?php endif; ?>
  <div class="islandora-document-sidebar">
    <?php if (!empty($dc_array['dc:description']['value'])): ?>
      <h2><?php print $dc_array['dc:description']['label']; ?></h2>
      <p><?php print $dc_array['dc:description']['value']; ?></p>
    <?php endif; ?>
    <?php if($parent_collections): ?>
      <div>
        <h2><?php print t('In collections'); ?></h2>
        <ul>
          <?php foreach ($parent_collections as $collection): ?>
            <li><?php print l($collection->label, "islandora/object/{$collection->id}"); ?></li>
          <?php endforeach; ?>
        </ul>
      </div>
    <?php endif; ?>
  </div>
  </div>
  <fieldset class="collapsible collapsed islandora-document-metadata">
  <legend><span class="fieldset-legend"><?php print t('Extended details'); ?></span></legend>
    <div class="fieldset-wrapper">
      <dl class="islandora-inline-metadata islandora-document-fields">
        <?php $row_field = 0; ?>
        <?php foreach ($dc_array as $key => $value): ?>
          <dt class="<?php print $value['class']; ?><?php print $row_field == 0 ? ' first' : ''; ?>">
            <?php print $value['label']; ?>
          </dt>
          <dd class="<?php print $value['class']; ?><?php print $row_field == 0 ? ' first' : ''; ?>">
            <?php print $value['value']; ?>
          </dd>
          <?php $row_field++; ?>
        <?php endforeach; ?>
      </dl>
    </div>
  </fieldset>
</div>
