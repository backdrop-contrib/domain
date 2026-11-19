Domain Access
=============
<!--
|<- - - - - - - This line is exactly 80 characters for reference - - - - - - ->|
-->
Domain Access lets one Backdrop site serve several domains (for example
`example.com`, `news.example.com` and `example.org`) from a single codebase
and database. Content, settings, themes and navigation can be shared between
domains or assigned to just one of them.

Common uses: a group of related sites that share some content, regional or
department sites managed by one team, or a network of small sites run from
one install.

This is a beta release. It has been in use on production sites for over a
year and the Drupal 7 automated tests have been ported. Please test it and
report bugs, and be careful before installing it on a production site.

Requirements
------------
- Every domain you plan to use must already reach this Backdrop site: DNS,
  web server aliases and the SSL certificate must cover all of them. Visiting
  each domain should show the same site before you install the module.
- A line added to `settings.php` (see Installation).

Installation
------------
1. Place the module in your `modules` directory (see
   https://docs.backdropcms.org/documentation/extend-with-modules). Don't
   enable it yet.
2. Add these lines to the **end** of `settings.php`, after the database
   settings and after any other file that `settings.php` includes:

   ```php
   /**
    * Add the domain module setup routine.
    */
   include BACKDROP_ROOT . '/modules/domain/settings.inc';
   ```

   Change `/modules/domain/` to match where you placed the module (for
   example `/modules/contrib/domain/`). This is the step most often missed:
   without it, every domain behaves like the primary domain.
3. Visit the site on your **primary** domain and enable the Domain Access
   module. The primary domain is created from the domain you are visiting.
   (With Bee: `bee en domain --base-url=https://example.com`.)
4. Rebuild content access permissions when prompted.

More detail, including troubleshooting: `INSTALL.txt`,
`INSTALL_QUICKSTART.txt` and the Quick Start page in the wiki:
https://github.com/backdrop-contrib/domain/wiki/2.-Domain-Quick-Start-Install

Basic configuration
-------------------
1. **Add your domains** at Administration > Structure > Domains
   (`admin/structure/domain`). Use "Create domain record" for each extra
   domain. Each domain gets a name and a hostname, and can be marked
   active or inactive.
2. **Review the settings** on the Settings tab
   (`admin/structure/domain/settings`), and on the Content defaults tab
   (`admin/structure/domain/nodes`) choose which domains each content type
   is published to by default.
3. **Set permissions** at Administration > Configuration > People >
   Permissions. Site administrators normally need "Administer domain
   records and settings" and "Set domain access status for all content".
   Editors who should work on only some domains need "Assign editors to
   domains" and "Edit any content on assigned domains".
4. **Assign people to domains** on each account's edit form. This controls
   which domains an editor can work on.
5. **Assign content to domains** on the node edit form, under "Domain access
   options": choose the domains where each piece of content appears, or
   publish it to all domains.
6. **Show blocks per domain** (optional): in a layout, add the "Domain"
   visibility condition to a block or to a whole layout.

Then try the optional sub-modules below for per-domain themes, settings and
navigation.

Sub-modules
-----------
All are optional and need the main Domain Access module. Enable only the
ones you need; each has its own README.txt.

- **Domain Alias**: lets one domain answer to several hostnames, including
  wildcards (for example `*.example.com`), and redirect aliases to the main
  hostname.
- **Domain Conf**: per-domain site settings: site name, slogan, email
  address, front page, default language and time zone.
- **Domain Content**: adds content overview pages for each domain, for
  reviewing and batch-editing content one domain at a time.
- **Domain Nav**: provides a block with links to each active domain, shown
  as a list, menu or select list.
- **Domain Settings**: adds a domain selector to many other settings forms,
  so their values can be saved for one domain instead of the whole site.
  Needs Domain Conf.
- **Domain Source**: sets a "source" domain for each piece of content, so
  links to it always point to one domain. Useful for SEO when content is
  shared.
- **Domain Strict**: makes content visible only to people assigned to that
  domain. Anonymous visitors see only content published to all domains.
- **Domain Theme**: assigns a different theme (and theme settings) to each
  domain.

Documentation
-------------
More documentation is in the wiki; please help us improve it:
https://github.com/backdrop-contrib/domain/wiki

Many Drupal 7 modules extended Domain Access. See the wiki for those already
available for Backdrop, and help us port more:
https://github.com/backdrop-contrib/domain/wiki/1.-Domain-module-wiki

Issues
------
Bugs and feature requests should be reported in the issue queue:
https://github.com/backdrop-contrib/domain/issues

AI Statement
------------
AI tools are being used in the ongoing development of this module.

Current Maintainers
-------------------
- [Tim Erickson](https://github.com/stpaultim) - [Simplo by Triplo](https://simplo.site/)
- [Rodobaldo Perez Macias](https://github.com/rudy880719)
- Collaboration and co-maintainers welcome! REALLY, REALLY WELCOME!!

Credits
-------
Port to Backdrop CMS by:
- [Rodobaldo Perez Macias](https://github.com/rudy880719)
- [Martin Price](https://github.com/yorkshire-pudding) - [System Horizons Ltd](https://www.systemhorizons.co.uk)
- [Tim Erickson](https://github.com/stpaultim) - [Simplo by Triplo](https://simplo.site/)

Drupal maintainers:
- [Ken Rickard (agentrickard)](https://www.drupal.org/u/agentrickard)
- [Katrin Valdre (nonsie)](https://www.drupal.org/u/nonsie)

Drupal development sponsored by [Palantir.net](https://palantir.net/)

Created for Drupal by [Gurpartap Singh](https://www.drupal.org/u/gurpartap-singh)

License
-------
This project is GPL v2 software.
See the LICENSE.txt file in this directory for complete text.
