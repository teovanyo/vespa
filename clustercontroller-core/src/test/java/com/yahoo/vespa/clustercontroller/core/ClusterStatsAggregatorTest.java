// Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.clustercontroller.core;

import com.google.common.collect.Sets;
import org.junit.jupiter.api.Test;

import java.util.Set;

import static org.junit.jupiter.api.Assertions.*;

/**
 * @author hakonhall
 */
public class ClusterStatsAggregatorTest {

    private static class Fixture {
        private final ClusterStatsAggregator aggregator;

        Fixture(Set<Integer> distributorNodes,
                Set<Integer> contentNodes) {
            aggregator = new ClusterStatsAggregator(distributorNodes, contentNodes);
        }

        void update(int distributorIndex, ContentClusterStatsBuilder clusterStats) {
            aggregator.updateForDistributor(distributorIndex, clusterStats.build());
        }

        public void verify(ContentClusterStatsBuilder expectedStats) {
            assertEquals(expectedStats.build(), aggregator.getAggregatedStats().getStats());
        }

        public void verify(int distributorIndex, ContentNodeStatsBuilder expectedStats) {
            assertEquals(expectedStats.build(), aggregator.getAggregatedStatsForDistributor(distributorIndex));
        }

        public void verifyGlobal(ContentNodeStatsBuilder expectedStats) {
            assertEquals(expectedStats.build(), aggregator.getAggregatedStats().getGlobalStats());
        }

        boolean hasUpdatesFromAllDistributors() {
            return aggregator.getAggregatedStats().hasUpdatesFromAllDistributors();
        }

    }

    private static class FourNodesFixture extends Fixture {
        FourNodesFixture() {
            super(distributorNodes(1, 2), contentNodes(3, 4));

            update(1, new ContentClusterStatsBuilder()
                    .add(3, "default", 10, 1)
                    .add(3, "global", 11, 2)
                    .add(4, "default", 12, 3)
                    .add(4, "global", 13, 4));
            update(2, new ContentClusterStatsBuilder()
                    .add(3, "default", 14, 5)
                    .add(3, "global", 15, 6)
                    .add(4, "default", 16, 7)
                    .add(4, "global", 17, 8));
        }
    }

    private static Set<Integer> distributorNodes(Integer... indices) {
        return Sets.newHashSet(indices);
    }

    private static Set<Integer> contentNodes(Integer... indices) {
        return Sets.newHashSet(indices);
    }

    private static ContentNodeStatsBuilder globalStatsBuilder() {
        return ContentNodeStatsBuilder.forNode(-1);
    }

    @Test
    void aggregator_handles_updates_to_single_distributor_and_content_node() {
        Fixture f = new Fixture(distributorNodes(1), contentNodes(3));
        ContentClusterStatsBuilder stats = new ContentClusterStatsBuilder()
                .add(3, "default", 10, 1)
                .add(3, "global", 11, 2);
        f.update(1, stats);
        f.verify(stats);
        f.verifyGlobal(globalStatsBuilder()
                .add("default", 10, 1)
                .add("global", 11, 2));
    }

    @Test
    void aggregator_handles_updates_to_multiple_distributors_and_content_nodes() {
        Fixture f = new FourNodesFixture();

        f.verify(new ContentClusterStatsBuilder()
                .add(3, "default", 10 + 14, 1 + 5)
                .add(3, "global",  11 + 15, 2 + 6)
                .add(4, "default", 12 + 16, 3 + 7)
                .add(4, "global",  13 + 17, 4 + 8));

        f.verifyGlobal(globalStatsBuilder()
                .add("default", (10 + 14) + (12 + 16), (1 + 5) + (3 + 7))
                .add("global",  (11 + 15) + (13 + 17), (2 + 6) + (4 + 8)));
    }

    @Test
    void aggregator_handles_multiple_updates_from_same_distributor() {
        Fixture f = new Fixture(distributorNodes(1, 2), contentNodes(3));

        f.update(1, new ContentClusterStatsBuilder().add(3, "default"));
        f.verify(new ContentClusterStatsBuilder().add(3, "default"));

        f.update(2, new ContentClusterStatsBuilder().add(3, "default", 10, 1));
        f.verify(new ContentClusterStatsBuilder().addInvalid(3, "default", 10, 1));
        f.verifyGlobal(globalStatsBuilder().addInvalid("default", 10, 1));

        f.update(1, new ContentClusterStatsBuilder().add(3, "default", 11, 2));
        f.verify(new ContentClusterStatsBuilder().add(3, "default", 10 + 11, 1 + 2));
        f.verifyGlobal(globalStatsBuilder().add("default", 10 + 11, 1 + 2));

        f.update(2, new ContentClusterStatsBuilder().add(3, "default", 15, 6));
        f.verify(new ContentClusterStatsBuilder().add(3, "default", 11 + 15, 2 + 6));
        f.verifyGlobal(globalStatsBuilder().add("default", 11 + 15, 2 + 6));

        f.update(1, new ContentClusterStatsBuilder().add(3, "default", 16, 7));
        f.verify(new ContentClusterStatsBuilder().add(3, "default", 15 + 16, 6 + 7));
        f.verifyGlobal(globalStatsBuilder().add("default", 15 + 16, 6 + 7));

        f.update(2, new ContentClusterStatsBuilder().add(3, "default", 12, 3));
        f.verify(new ContentClusterStatsBuilder().add(3, "default", 16 + 12, 7 + 3));
        f.verifyGlobal(globalStatsBuilder().add("default", 16 + 12, 7 + 3));
    }

    @Test
    void aggregator_handles_more_content_nodes_than_distributors() {
        Fixture f = new Fixture(distributorNodes(1), contentNodes(3, 4));
        ContentClusterStatsBuilder stats = new ContentClusterStatsBuilder()
                .add(3, "default", 10, 1)
                .add(4, "default", 11, 2);
        f.update(1, stats);
        f.verify(stats);
        f.verifyGlobal(globalStatsBuilder().add("default", 10 + 11, 1 + 2));
    }

    @Test
    void aggregator_ignores_updates_to_unknown_distributor() {
        Fixture f = new Fixture(distributorNodes(1), contentNodes(3));
        final int downDistributorIndex = 2;
        f.update(downDistributorIndex, new ContentClusterStatsBuilder()
                .add(3, "default", 7, 3));
        f.verify(new ContentClusterStatsBuilder().add(3));
    }

    @Test
    void aggregator_tracks_when_it_has_updates_from_all_distributors() {
        Fixture f = new Fixture(distributorNodes(1, 2), contentNodes(3));
        assertFalse(f.hasUpdatesFromAllDistributors());
        f.update(1, new ContentClusterStatsBuilder().add(3, "default"));
        assertFalse(f.hasUpdatesFromAllDistributors());
        f.update(1, new ContentClusterStatsBuilder().add(3, "default", 10, 1));
        assertFalse(f.hasUpdatesFromAllDistributors());
        f.update(2, new ContentClusterStatsBuilder().add(3, "default"));
        assertTrue(f.hasUpdatesFromAllDistributors());
    }

    @Test
    void aggregator_can_provide_aggregated_stats_per_distributor() {
        Fixture f = new FourNodesFixture();

        f.verify(1, ContentNodeStatsBuilder.forNode(1)
                .add("default", 10 + 12, 1 + 3)
                .add("global", 11 + 13, 2 + 4));

        f.verify(2, ContentNodeStatsBuilder.forNode(2)
                .add("default", 14 + 16, 5 + 7)
                .add("global", 15 + 17, 6 + 8));
    }

    @Test
    void aggregator_tracks_total_document_count_and_byte_size_across_distributors() {
        Fixture f = new Fixture(distributorNodes(0, 1), contentNodes(0));
        f.update(0, new ContentClusterStatsBuilder().add(0, "default").withDocumentCountTotal(100).withBytesTotal(2000));
        assertFalse(f.hasUpdatesFromAllDistributors());
        f.update(1, new ContentClusterStatsBuilder().add(0, "default").withDocumentCountTotal(200).withBytesTotal(3000));
        assertTrue(f.hasUpdatesFromAllDistributors());

        assertEquals(300, f.aggregator.getAggregatedDocumentCountTotal());
        assertEquals(5000, f.aggregator.getAggregatedBytesTotal());

        f.update(0, new ContentClusterStatsBuilder().add(0, "default").withDocumentCountTotal(150).withBytesTotal(2020));
        assertTrue(f.hasUpdatesFromAllDistributors());
        assertEquals(350, f.aggregator.getAggregatedDocumentCountTotal());
        assertEquals(5020, f.aggregator.getAggregatedBytesTotal());

        f.update(1, new ContentClusterStatsBuilder().add(0, "default").withDocumentCountTotal(210).withBytesTotal(2900));
        assertEquals(360, f.aggregator.getAggregatedDocumentCountTotal());
        assertEquals(4920, f.aggregator.getAggregatedBytesTotal());
    }

}
