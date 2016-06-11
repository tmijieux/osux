
SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

-- --------------------------------------------------------
--
-- Table
--

CREATE TABLE `tr_beatmap_set` (
  `ID` int(11) NOT NULL,
  `artist_uni` varchar(256) DEFAULT NULL,
  `artist` varchar(256) NOT NULL,
  `title_uni` varchar(256) DEFAULT NULL,
  `title` varchar(256) NOT NULL,
  `source` varchar(256) DEFAULT NULL,
  `creator_ID` int(11) NOT NULL,
  `osu_map_ID` int(11) DEFAULT NULL COMMENT 'copied from osu'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

CREATE TABLE `tr_diff` (
  `ID` int(11) NOT NULL,
  `bms_ID` int(11) DEFAULT NULL,
  `osu_diff_ID` int(11) DEFAULT NULL COMMENT 'copied from osu',
  `hash` varchar(48) NOT NULL,
  `diff_name` varchar(256) NOT NULL,
  `max_combo` int(11) NOT NULL,
  `bonus` int(11) NOT NULL,
  `last_update` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

CREATE TABLE `tr_mod` (
  `ID` int(11) NOT NULL,
  `mod_name` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

CREATE TABLE `tr_perf` (
  `user_ID` int(11) NOT NULL,
  `score_ID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

CREATE TABLE `tr_score` (
  `ID` int(11) NOT NULL,
  `diff_ID` int(11) NOT NULL,
  `mod_ID` int(11) NOT NULL,
  `accuracy` double NOT NULL,
  `combo` int(11) NOT NULL,
  `great` int(11) NOT NULL,
  `good` int(11) NOT NULL,
  `miss` int(11) NOT NULL,
  `final_star` double NOT NULL,
  `density_star` double NOT NULL,
  `pattern_star` double NOT NULL,
  `reading_star` double NOT NULL,
  `accuracy_star` double NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

CREATE TABLE `tr_user` (
  `ID` int(11) NOT NULL,
  `name` varchar(256) NOT NULL,
  `osu_user_ID` int(11) DEFAULT NULL,
  `density_star` double NOT NULL,
  `reading_star` double NOT NULL,
  `pattern_star` double NOT NULL,
  `accuracy_star` double NOT NULL,
  `final_star` double NOT NULL,
  `rank` int(11) DEFAULT NULL,
  `rank_country` int(11) DEFAULT NULL,
  `last_update` datetime DEFAULT NULL,
  `country` varchar(8) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------
--
-- key
--

ALTER TABLE `tr_beatmap_set`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `creator_ID` (`creator_ID`);

ALTER TABLE `tr_diff`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `beatmap_set_ID` (`bms_ID`);

ALTER TABLE `tr_mod`
  ADD PRIMARY KEY (`ID`);

ALTER TABLE `tr_perf`
  ADD PRIMARY KEY (`user_ID`,`score_ID`),
  ADD KEY `FK_PERF_SCORE` (`score_ID`),
  ADD KEY `user_ID` (`user_ID`);

ALTER TABLE `tr_score`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `diff_ID` (`diff_ID`),
  ADD KEY `mod_ID` (`mod_ID`);

ALTER TABLE `tr_user`
  ADD PRIMARY KEY (`ID`);

-- --------------------------------------------------------
--
-- AUTO_INCREMENT
--

ALTER TABLE `tr_beatmap_set`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=0;

ALTER TABLE `tr_diff`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=0;

ALTER TABLE `tr_mod`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=0;

ALTER TABLE `tr_score`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=0;

ALTER TABLE `tr_user`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=0;

-- --------------------------------------------------------
--
-- Constraint
--

ALTER TABLE `tr_beatmap_set`
  ADD CONSTRAINT `FK_BEATMAP_SET_USER` FOREIGN KEY (`creator_ID`) REFERENCES `tr_user` (`ID`);

ALTER TABLE `tr_diff`
  ADD CONSTRAINT `FK_DIFF_BEATMAP_SET` FOREIGN KEY (`bms_ID`) REFERENCES `tr_beatmap_set` (`ID`);

ALTER TABLE `tr_perf`
  ADD CONSTRAINT `FK_PERF_SCORE` FOREIGN KEY (`score_ID`) REFERENCES `tr_score` (`ID`),
  ADD CONSTRAINT `FK_PERF_USER` FOREIGN KEY (`user_ID`) REFERENCES `tr_user` (`ID`);

ALTER TABLE `tr_score`
  ADD CONSTRAINT `FK_SCORE_DIFF` FOREIGN KEY (`diff_ID`) REFERENCES `tr_diff` (`ID`),
  ADD CONSTRAINT `FK_SCORE_MOD` FOREIGN KEY (`mod_ID`) REFERENCES `tr_mod` (`ID`);
