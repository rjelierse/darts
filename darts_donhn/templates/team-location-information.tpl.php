<?php
/**
 * Darts DONHN competition module: Team location information template.
 *
 * Available variables:
 *  - $name: The name of the location.
 *  - $address: The address of the location.
 *  - $city: The city of the location.
 *  - $phone: The phone number of the location.
 */
?>

<?php echo $name; ?>
<?php echo $address; ?>
<?php echo $city; ?>
<?php if ($phone): ?>
<span class="contact-phone"><?php echo $phone; ?></span>
<?php endif; ?>