<?php
/**
 * Darts competition module: Result template.
 *
 * Available variables:
 *  - $result: Raw result data.
 *  - $result_date: The date the result was set (formatted).
 *  - $home_player: Formatted link to the home player's profile.
 *  - $home_player_classes: Extra classes for the home player.
 *  - $away_player: Formatted link to the away player's profile.
 *  - $away_player_classes: Extra classes for the away player.
 *
 * @todo Add statistics.
 */
?>

<div class="result competition-result">
    <div class="date"><?php echo $result_date; ?></div>

    <h2 class="section-title"><?php echo t('Result'); ?></h2>
    <div class="scoreboard clear-block">
        <div class="player home-player <?php echo $home_player_classes; ?>">
            <?php echo $home_player; ?>
            <div class="score"><?php echo $result['home_legs']; ?></div>
        </div>
        <div class="player away-player <?php echo $away_player_classes; ?>">
            <?php echo $away_player; ?>
            <div class="score"><?php echo $result['away_legs']; ?></div>
        </div>
    </div>

    <!-- // Hide this, as it is not yet used.
    <h2 class="section-title"><?php echo t('Statistics'); ?></h2>
    <div class="statistics">
        <?php echo $statistics; ?>
    </div>
    // -->
</div>