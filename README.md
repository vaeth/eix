<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
   <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
   <title>eix - search util for Gentoo's portage</title></title>
</head>
<body>
<!-- content -->
<table BORDER=0 CELLSPACING=0 CELLPADDING=0 WIDTH="100%" >
<tr VALIGN=TOP>
<td align=center>
<h2>Welcome to</h2>
<img alt="eix" title="The wild eix fox!" id="fox" src="eix.png">
</td>
</tr>
</table>
<table>
<tr VALIGN=TOP>
<td>
<p>eix is a tool for the <a href="http://gentoo.org/">Gentoo Linux</a>
portage system. It uses indexing to allow quick searches through the package tree.
Some reasons for using eix:
<ul>
	<li>Fast indexing and searching</li>
	<li>Highly configurable output-format (with its own "language")</li>
	<li>Provides more information than most other search utilities (e.g. shows every available version, USE flags of available and emerged versions, ...)</li>
	<li>Shows stability/mask status of versions and how your local settings change it</li>
	<li>Helps you to manage your local /etc/portage/package.* settings in several ways (e.g. by finding "obsolete" entries by various criteria)</li>
	<li>Can import eix-files from other systems, e.g. to get a catalogue of overlays</li>
	<li>Cli with logical AND/OR (and braces), multiple matching algorithms (i.e. fuzzy search using levenshtein distance) and more</li>
</ul>
<p>eix is in the portage tree, so you can simply install it with
<p><center><tt>emerge app-portage/eix</tt></center>
<p>The most current version of eix is always available as "HEAD" in the git master branch.
To get this version, emerge dev-vcs/git and then use the command
<p><center><tt>git clone git://github.com/vaeth/eix</tt></center>
<p>For bugreports, please use the GitHub or the Gentoo bug system.
If you want to reach the current maintainer, please use email.

<p>Best regards .. your code monkeys :)

<h3>Other search-utils</h3>
<ul>
	<li>emerge -s|-S</li>
	<li>qsearch (app-portage/portage-utils)</li>
	<li>esearch (app-portage/esearch)</li>
</ul>
</td>
</tr>
</table>
<!-- end content -->

</body>
</html>
