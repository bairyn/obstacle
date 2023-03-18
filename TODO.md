- 2023-03-18 Double check oc mode based config load, as sometimes it seems oc
  needs to be loaded twice rather than once for the motd to be updated.
- 2023-03-18 UI Game -> Map -> layouts seems to work for ‘callvote atcs oc’ but
  not for `*BUILTIN*`.
- 2023-03-18 Dretch on atcs ochd seemed to have bugged wallwalk angles once it
  starts to climb the wall in the beginning room, climbing up near the DCs.
- 2023-03-18 in editoc mode, probably by default assume `g_markDeconstruct` is
  disabled unless it's 2 or some other setting explicitly enables it.
- 2023-03-18 Organize OC mode, dom mode, and standard mode, and make sure each
  works.  Maybe have separate branches, one for upstream / standard, one for
  dom-only, one for oc-only, and one for combined, and alias a branch, likely
  combined (supports both OC and dom (and standard)) as named ‘master’.  (Also
  figure out how ‘i18n’ should be organized here.)
- 2023-03-18 perhaps finish a GPP-compatible 2.0 version, and fork to a new OC
  v2.1 version compatible with Tremulous 1.2.  Perhaps you can fork it from
  upstream (darklegion) git and clean it up / organize it.  (IIRC it's now
  protocol 71?  Get up to date.)
- 2023-03-18 Double check i18n works and fix it for OC 2.0.  See if it's still
  useful for 2.1.
- 2023-03-18 (This or equivalent, and document) add a flag for 1.2 gameplay
  mechanics / physics (e.g. jumping from atcs ramps, turret behavior, bounding
  boxes, marauder jumping, etc.), as this mod started with 1.1 mechanics and
  even though it's been upgraded to GPP, the mechanics in _oc mode_ currently
  is 1.1 mechanics only (but switches back to GPP mechanics in non-oc mode).
  Perhaps encourage this flag in development for new courses and discourage in
  new courses.  (A few ideas to maybe think about: maybe have a flag to
  explicitly have 1.1 mechanics even though it's the default; maybe have a
  builtin list of old layouts grandfathered in with 1.1 defaults (probably not,
  though; too confusing maybe); or warn if the layout before `_` doesn't end in either
  ‘-1.1’ or ‘-1.2’, saying it'll default to 1.1 mechanics.)
- 2023-03-18 for editoc granger on atcs in the middle, acid tube blueprints
  that the client seemed half buried in the ground, but it might still build
  just fine and as expected, but double check especially that it builds as
  expected, and second and less importantly it also would probably be nice for
  the blueprint to match up as expected too.
