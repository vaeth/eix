<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
   <meta http-equiv="Content-Type" content="text/html;charset=utf-8" />
   <!-- <title>eix - search util for Gentoo's portage</title> -->
</head>
<body>
<!-- content -->
<table>
<tr>
<td>
<h2>Welcome to</h2>
<img alt="eix" title="The wild eix fox!" id="fox" src="eix.png" />
</td>
</tr>
</table>
<table>
<tr>
<td>
<p>eix is a tool for the <a href="http://gentoo.org/">Gentoo Linux</a>
portage system. It uses indexing to allow quick searches through the package tree.
Some reasons for using eix:</p>
<ul>
	<li>Fast indexing and searching</li>
	<li>Highly configurable output-format (with its own "language")</li>
	<li>Provides more information than most other search utilities (e.g. shows every available version, USE flags of available and emerged versions, ...)</li>
	<li>Shows stability/mask status of versions and how your local settings change it</li>
	<li>Helps you to manage your local /etc/portage/package.* settings in several ways (e.g. by finding "obsolete" entries by various criteria)</li>
	<li>Can import eix-files from other systems, e.g. to get a catalogue of overlays</li>
	<li>Cli with logical AND/OR (and braces), multiple matching algorithms (i.e. fuzzy search using levenshtein distance) and more</li>
</ul>
<p>eix is in the portage tree, so you can simply install it with</p>
<p><tt>emerge app-portage/eix</tt></p>
<p>The most current version of eix is always available as "HEAD" in the git master branch.
To get this version, emerge dev-vcs/git and then use the command</p>
<p><tt>git clone git://github.com/vaeth/eix</tt></p>
<p>For bugreports, please use the GitHub or the Gentoo bug system.
If you want to reach the current maintainer, please use email.</p>

<p>Best regards .. your code monkeys :)</p>

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
