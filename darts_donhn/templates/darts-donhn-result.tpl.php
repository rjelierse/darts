<?php
/**
 * Darts DONHN competition module: Result template.
 *
 * Available variables:
 *  - $result_date: The date the result was set (formatted).
 *  - $home_team: Team that played at home.
 *  - $home_team_score: Score for the home team.
 *  - $home_team_classes: Classes for the home team.
 *  - $away_team: Team that played away.
 *  - $away_team_score: Score for the away team.
 *  - $away_team_classes: Classes for the away team.
 *
 * @todo Add statistics.
 */
?>

<div class="result donhn-result">
    <div class="date"><?php echo t('Played on %date', array('%date' => $result_date)); ?></div>

    <h2 class="section-title"><?php echo t('Result'); ?></h2>
    <div class="scoreboard clear-block">
        <div class="team home-team <?php echo $home_team_classes; ?>">
            <?php echo $home_team; ?>
            <div class="score"><?php echo $home_team_score; ?></div>
        </div>
        <div class="team away-team <?php echo $away_team_classes; ?>">
            <?php echo $away_team; ?>
            <div class="score"><?php echo $away_team_score; ?></div>
        </div>
    </div>

    <!-- // Hide this, as it is not yet used.
    <h2 class="section-title"><?php echo t('Statistics'); ?></h2>
    <div class="statistics">
        <?php echo $statistics; ?>
    </div>
    // -->
</div>
 
